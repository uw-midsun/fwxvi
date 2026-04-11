/************************************************************************************************
 * @file   cell_sense.c
 *
 * @brief  Source code for AFE cell sense
 *
 * @date   2025-01-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "adbms_afe.h"
#include "can.h"
#include "delay.h"
#include "global_enums.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "spi.h"
#include "system_can.h"
#include "tasks.h"

/* Intra-component Headers */
#include "bps_fault.h"
#include "cell_sense.h"
#include "rear_controller.h"
#include "rear_controller_getters.h"
#include "rear_controller_hw_defs.h"
#include "rear_controller_safety_limits.h"
#include "rear_controller_setters.h"
#include "rear_controller_state_manager.h"
#include "relays.h"
#include "thermistor.h"

#pragma GCC diagnostic ignored "-Wunused-function"

/************************************************************************************************
 * Private defines
 ************************************************************************************************/

/** @brief  Number of AFE messages */
#define NUM_AFE_MSGS 4U

/** @brief  Number of cell voltages per device transmitted in a single log */
#define NUM_AFE_CELL_VOLTAGES_PER_LOG 6U

/** @brief  Number of temperatures transmitted in a single log */
#define NUM_AFE_TEMPERATURES_PER_LOG 7U

/** @brief  Number of communication retries before throwing AFE fault */
#define AFE_NUM_RETRIES 5U

/** @brief  Temperature threshold for invalid readings */
#define CELL_TEMP_OUTLIER_THRESHOLD 80

/** @brief  Maximum pack current for cell discharging current - 7.0A -> 7000mA */
#define MAX_PACK_CURRENT_FOR_CELL_DISCHARGING 7000

/** @brief  Private define to lookup cell voltage */
#define CELL_PER_DEVICE (ADBMS_AFE_MAX_CELLS_PER_DEVICE)

#define CELL_VOLTAGE_LOOKUP(dev_num, cell) (adbms_afe_storage->cell_voltages[adbms_afe_storage->cell_result_lookup[CELL_PER_DEVICE * dev_num + cell]])

/** @brief  Max number of retries for reading cell*/
#define CELL_SENSE_MAX_RETRIES 10U

#define RETRY_OPERATION(max_retries, delay_ms_val, operation, status_var) \
  do {                                                                    \
    uint8_t _retries_left = (max_retries);                                \
    (status_var) = STATUS_CODE_INTERNAL_ERROR;                            \
    while (_retries_left-- > 0) {                                         \
      (status_var) = (operation);                                         \
      if ((status_var) == STATUS_CODE_OK) break;                          \
      delay_ms(delay_ms_val);                                             \
    }                                                                     \
  } while (0)

#define THERMISTORS_CONNECTED 0U
#define BALANCING_ENABLED 0U
#define OVER_UNDER_FAULTS_ENABLED 0U

#define CELL_SENSE_DEBUG 0U

#if (CELL_SENSE_DEBUG == 1)
#define CONDITIONAL_LOG_DEBUG(...) LOG_DEBUG(__VA_ARGS__)
#else
#define CONDITIONAL_LOG_DEBUG(...) \
  do {                             \
  } while (0)
#endif

/************************************************************************************************
 * Private variables
 ************************************************************************************************/

static SpiSettings adbms_spi_config = {
  .baudrate = REAR_CONTROLLER_ADBMS_AFE_SPI_BAUDRATE,
  .mode = SPI_MODE_3,
  .sdo = GPIO_REAR_CONTROLLER_ADBMS_AFE_SPI_SDO_GPIO,
  .sdi = GPIO_REAR_CONTROLLER_ADBMS_AFE_SPI_SDI_GPIO,
  .sclk = GPIO_REAR_CONTROLLER_ADBMS_AFE_SPI_SCK_GPIO,
  .cs = GPIO_REAR_CONTROLLER_ADBMS_AFE_SPI_CS_GPIO,
};

static const AdbmsAfeSettings s_afe_settings = {
  .spi_settings = &adbms_spi_config,

  .spi_port = REAR_CONTROLLER_ADBMS_AFE_SPI_PORT,

  .adc_mode = ADBMS_AFE_ADC_MODE_7KHZ,

  .num_devices = ADBMS_AFE_MAX_DEVICES,
  .num_cells = ADBMS_AFE_MAX_CELLS_PER_DEVICE,
  .num_thermistors = ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE,
};

static AdbmsAfeStorage *adbms_afe_storage;

static bool s_cell_data_updated = false;

static uint8_t s_afe_message_index = 0U;

static RearControllerStorage *rear_controller_storage;

static uint8_t s_global_cell_index_1_based(uint8_t device, uint8_t cell) {
  return (uint8_t)(device * ADBMS_AFE_MAX_CELLS_PER_DEVICE + cell + 1U);
}

static uint8_t s_global_thermistor_index_1_based(uint8_t device, uint8_t thermistor) {
  return (uint8_t)(device * ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE + thermistor + 1U);
}

/************************************************************************************************
 * Private function definitions
 ************************************************************************************************/

static void s_balance_cells(uint16_t min_voltage) {
#if (BALANCING_ENABLED == 1U)
  uint16_t balancing_threshold = min_voltage;

  if (rear_controller_storage->pack_current > MAX_PACK_CURRENT_FOR_CELL_DISCHARGING) {
    return;
  }

  /* Adjust balancing threshold */
  if (min_voltage >= AFE_BALANCING_UPPER_THRESHOLD) {
    /* If the minimum cell voltage is greater than 4.15V, we want minimum 2mV imbalance */
    balancing_threshold += 20U;
  } else if (min_voltage < AFE_BALANCING_UPPER_THRESHOLD && balancing_threshold >= AFE_BALANCING_LOWER_THRESHOLD) {
    /* If the minimum cell voltage is between 4.0V and 4.15V, we want minimum 5mV imbalance */
    balancing_threshold += 50U;
  } else {
    /* If the minimum cell voltage is less than 4.0V, we want minimum 10mV imbalance */
    balancing_threshold += 100U;
  }

  /* Toggle cell discharge in the ADBMS1818 configuration if cell voltage is above the balancing threshold */
  for (size_t dev = 0U; dev < s_afe_settings.num_devices; dev++) {
    for (size_t cell = 0U; cell < s_afe_settings.num_cells; cell++) {
      uint16_t global_cell = (uint16_t)(cell + (dev * ADBMS_AFE_MAX_CELLS_PER_DEVICE));
      if (CELL_VOLTAGE_LOOKUP(dev, cell) > balancing_threshold) {
        LOG_DEBUG("DISCHRG CELL %d %d\r\n", (uint8_t)dev, (uint8_t)cell);
        delay_ms(12U);
        adbms_afe_toggle_cell_discharge(adbms_afe_storage, global_cell, true);
      } else {
        adbms_afe_toggle_cell_discharge(adbms_afe_storage, global_cell, false);
      }
    }
  }

  /* Commit the discharge configuration to the ADBMS1818 */
  adbms_afe_write_config(adbms_afe_storage);
#endif
}

static void s_disable_balancing() {
  /* Toggle cell discharge in the ADBMS1818 configuration if cell voltage is above the balancing threshold */
  for (size_t dev = 0U; dev < s_afe_settings.num_devices; dev++) {
    for (size_t cell = 0U; cell < s_afe_settings.num_cells; cell++) {
      uint16_t global_cell = (uint16_t)(cell + (dev * ADBMS_AFE_MAX_CELLS_PER_DEVICE));
      adbms_afe_toggle_cell_discharge(adbms_afe_storage, global_cell, false);
    }
  }

  /* Commit the discharge configuration to the ADBMS1818 */
  adbms_afe_write_config(adbms_afe_storage);
}

static StatusCode s_check_thermistors() {
  StatusCode status = STATUS_CODE_OK;
#if (THERMISTORS_CONNECTED == 1U)
  uint16_t max_temp = 0U;

  /* Loop over all devices and thermistors */
  for (uint8_t device = 0U; device < s_afe_settings.num_devices; device++) {
    for (uint8_t thermistor = 0U; thermistor < ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE; thermistor++) {
      uint8_t index = device * ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE + thermistor;
      adbms_afe_storage->thermistor_voltages[index] = calculate_board_thermistor_temperature(adbms_afe_storage->thermistor_voltages[index] / 10U);

      /* Ignore temperature readings outside of the valid temperature range */
      if (adbms_afe_storage->thermistor_voltages[index] > CELL_TEMP_OUTLIER_THRESHOLD) {
        continue;
      }

      if (adbms_afe_storage->thermistor_voltages[index] > max_temp) {
        max_temp = adbms_afe_storage->thermistor_voltages[index];
      }

      if (rear_controller_storage->pack_current < 0) {
        /* Discharging max temp */
        if (adbms_afe_storage->thermistor_voltages[index] >= CELL_OVERTEMP_DISCHARGE_LIMIT_C) {
          LOG_DEBUG("CELL OVERTEMP\n");
          trigger_bps_fault_with_cell(BPS_FAULT_OVERTEMP_CELL, s_global_thermistor_index_1_based(device, thermistor));
          status = STATUS_CODE_INTERNAL_ERROR;
        }
      } else {
        /* Charging max temp */
        if (adbms_afe_storage->thermistor_voltages[index] >= CELL_OVERTEMP_CHARGE_LIMIT_C) {
          LOG_DEBUG("CELL OVERTEMP\n");
          trigger_bps_fault_with_cell(BPS_FAULT_OVERTEMP_CELL, s_global_thermistor_index_1_based(device, thermistor));
          status = STATUS_CODE_INTERNAL_ERROR;
        }
      }
    }
  }
#endif
  return status;
}

static StatusCode s_cell_sense_conversions() {
  StatusCode status = STATUS_CODE_OK;

  for (uint8_t retries = AFE_NUM_RETRIES; retries > 0; retries--) {
    RETRY_OPERATION(CELL_SENSE_MAX_RETRIES, RETRY_DELAY_MS, adbms_afe_trigger_cell_conv(adbms_afe_storage), status);

    if (status == STATUS_CODE_OK) {
      /* If conversion triggered successfully, escape early */
      break;
    }

    LOG_DEBUG("Cell trigger conv failed, retrying %d\n", status);
    delay_ms(RETRY_DELAY_MS);
  }

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Cell conv failed): %d\n", status);
#if (OVER_UNDER_FAULTS_ENABLED == 1)
    trigger_bps_fault(BPS_FAULT_COMMS_LOSS_AFE);
#endif
    return status;
  }

  delay_ms(CONV_DELAY_MS);

  for (uint8_t retries = AFE_NUM_RETRIES; retries > 0; retries--) {
    status = adbms_afe_read_cells(adbms_afe_storage);
    if (status == STATUS_CODE_OK) {
      break;
    }
    LOG_DEBUG("Cell read failed, retrying %d\n", status);
    delay_ms(RETRY_DELAY_MS);
  }
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Cell read failed %d\n", status);
#if (OVER_UNDER_FAULTS_ENABLED == 1)
    trigger_bps_fault(BPS_FAULT_COMMS_LOSS_AFE);
#endif
    return status;
  }
#if (THERMISTORS_CONNECTED == 1U)
  for (uint8_t retries = AFE_NUM_RETRIES; retries > 0; retries--) {
    status = adbms_afe_trigger_thermistor_conv(adbms_afe_storage);

    if (status == STATUS_CODE_OK) {
      break;
    }

    LOG_DEBUG("Aux trigger conv failed, retrying: %d\n", status);
    delay_ms(RETRY_DELAY_MS);
  }

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Thermistor conv failed: Status %d\n", status);
    trigger_bps_fault(BPS_FAULT_COMMS_LOSS_AFE);
    return status;
  }

  delay_ms(AUX_CONV_DELAY_MS);

  for (uint8_t retries = AFE_NUM_RETRIES; retries > 0; retries--) {
    status = adbms_afe_read_thermistors(adbms_afe_storage);

    if (status == STATUS_CODE_OK) {
      break;
    }

    LOG_DEBUG("Thermistor read failed, retrying %d\n", status);
    delay_ms(RETRY_DELAY_MS);
  }

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Thermistor read failed: %d\n", status);
    trigger_bps_fault(BPS_FAULT_COMMS_LOSS_AFE);
    return status;
  }
#endif
  return status;
}

static StatusCode s_cell_sense_run() {
  StatusCode status = STATUS_CODE_OK;

  uint16_t max_voltage = 0U;
  uint16_t min_voltage = 0xFFFFU;
#if (OVER_UNDER_FAULTS_ENABLED == 1)
  uint8_t max_voltage_cell = 0U;
  uint8_t min_voltage_cell = 0U;
#endif
  uint32_t total_voltage = 0;

  for (size_t dev = 0U; dev < s_afe_settings.num_devices; dev++) {
    for (size_t cell = 0U; cell < s_afe_settings.num_cells; cell++) {
      uint16_t current_cell_voltage = (uint16_t)CELL_VOLTAGE_LOOKUP(dev, cell);
      total_voltage += current_cell_voltage;
      CONDITIONAL_LOG_DEBUG("CELL %d %d: %d\r\n", (uint8_t)dev, (uint8_t)cell, current_cell_voltage);
      delay_ms(12U);

      if (current_cell_voltage > max_voltage) {
        max_voltage = current_cell_voltage;
#if (OVER_UNDER_FAULTS_ENABLED == 1)
        max_voltage_cell = s_global_cell_index_1_based((uint8_t)dev, (uint8_t)cell);
#endif
      }

      if (current_cell_voltage < min_voltage) {
        min_voltage = current_cell_voltage;
#if (OVER_UNDER_FAULTS_ENABLED == 1)
        min_voltage_cell = s_global_cell_index_1_based((uint8_t)dev, (uint8_t)cell);
#endif
      }
    }
  }

  rear_controller_storage->pack_voltage = total_voltage / 10000;

  CONDITIONAL_LOG_DEBUG("PACK V: %lu\r\n", rear_controller_storage->pack_voltage);
  delay_ms(10U);
  CONDITIONAL_LOG_DEBUG("MAX VOLTAGE: %d\r\nMIN VOLTAGE: %d\r\nUNBALANCE: %d\r\n", max_voltage, min_voltage, max_voltage - min_voltage);
  delay_ms(10U);

  set_battery_stats_B_max_cell_voltage(max_voltage);
  set_battery_stats_B_min_cell_voltage(min_voltage);

  if (max_voltage >= SOLAR_VOLTAGE_THRESHOLD) {
    relays_open_solar();
  }

  /*
   * Compare max and min voltages to safety limits
   * We must multiply the safety limit of 10 to convert from mV -> 100 uV
   */
  if (max_voltage >= (CELL_OVERVOLTAGE_LIMIT_mV * 10U)) {
    LOG_DEBUG("FAULT: OVERVOLTAGE: %u\r\n", max_voltage);
#if (OVER_UNDER_FAULTS_ENABLED == 1)
    trigger_bps_fault_with_cell(BPS_FAULT_OVERVOLTAGE, max_voltage_cell);
#endif
    status = STATUS_CODE_INTERNAL_ERROR;
  }

  if (min_voltage <= (CELL_UNDERVOLTAGE_LIMIT_mV * 10U)) {
    LOG_DEBUG("FAULT: UNDERVOLTAGE: %u\r\n", min_voltage);
#if (OVER_UNDER_FAULTS_ENABLED == 1)
    trigger_bps_fault_with_cell(BPS_FAULT_UNDERVOLTAGE, min_voltage_cell);
#endif
    status = STATUS_CODE_INTERNAL_ERROR;
  }

  if ((max_voltage - min_voltage) >= (CELL_UNBALANCED_LIMIT_mV * 10)) {
    /* Note (From Aryan): We don't actually need to fault on imbalance. It is here for safety. Remove if needed */
    LOG_DEBUG("FAULT: UNBALANCED: %u\r\n", max_voltage - min_voltage);
#if (OVER_UNDER_FAULTS_ENABLED == 1)
    trigger_bps_fault_with_cell(BPS_FAULT_UNBALANCE, max_voltage_cell);
#endif
    status = STATUS_CODE_INTERNAL_ERROR;
  }

#if (BALANCING_ENABLED == 1U)
  s_balance_cells(min_voltage);
#endif
  s_cell_data_updated = true;

#if (THERMISTORS_CONNECTED == 1U)
  status_ok_or_return(s_check_thermistors());
#endif

  return status;
}

/************************************************************************************************
 * Public function definitions
 ************************************************************************************************/

StatusCode log_cell_sense() {
  /* AFE messages are split into 3 (For each AFE) */
  /* We send 4 messages total to transmit all cell voltages (3 per msg * 4 times) = 12 total cell voltages */
  if (s_cell_data_updated != true) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  set_AFE1_status_A_id(s_afe_message_index);
  set_AFE1_status_A_voltage_0(CELL_VOLTAGE_LOOKUP(0U, (s_afe_message_index * NUM_AFE_CELL_VOLTAGES_PER_LOG) + 0U));
  set_AFE1_status_A_voltage_1(CELL_VOLTAGE_LOOKUP(0U, (s_afe_message_index * NUM_AFE_CELL_VOLTAGES_PER_LOG) + 1U));
  set_AFE1_status_A_voltage_2(CELL_VOLTAGE_LOOKUP(0U, (s_afe_message_index * NUM_AFE_CELL_VOLTAGES_PER_LOG) + 2U));

  set_AFE1_status_B_id(s_afe_message_index);
  set_AFE1_status_B_voltage_0(CELL_VOLTAGE_LOOKUP(0U, (s_afe_message_index * NUM_AFE_CELL_VOLTAGES_PER_LOG) + 3U));
  set_AFE1_status_B_voltage_1(CELL_VOLTAGE_LOOKUP(0U, (s_afe_message_index * NUM_AFE_CELL_VOLTAGES_PER_LOG) + 4U));
  set_AFE1_status_B_voltage_2(CELL_VOLTAGE_LOOKUP(0U, (s_afe_message_index * NUM_AFE_CELL_VOLTAGES_PER_LOG) + 5U));

  set_AFE2_status_A_id(s_afe_message_index);
  set_AFE2_status_A_voltage_0(CELL_VOLTAGE_LOOKUP(1U, (s_afe_message_index * NUM_AFE_CELL_VOLTAGES_PER_LOG) + 0U));
  set_AFE2_status_A_voltage_1(CELL_VOLTAGE_LOOKUP(1U, (s_afe_message_index * NUM_AFE_CELL_VOLTAGES_PER_LOG) + 1U));
  set_AFE2_status_A_voltage_2(CELL_VOLTAGE_LOOKUP(1U, (s_afe_message_index * NUM_AFE_CELL_VOLTAGES_PER_LOG) + 2U));

  set_AFE2_status_B_id(s_afe_message_index);
  set_AFE2_status_B_voltage_0(CELL_VOLTAGE_LOOKUP(1U, (s_afe_message_index * NUM_AFE_CELL_VOLTAGES_PER_LOG) + 3U));
  set_AFE2_status_B_voltage_1(CELL_VOLTAGE_LOOKUP(1U, (s_afe_message_index * NUM_AFE_CELL_VOLTAGES_PER_LOG) + 4U));
  set_AFE2_status_B_voltage_2(CELL_VOLTAGE_LOOKUP(1U, (s_afe_message_index * NUM_AFE_CELL_VOLTAGES_PER_LOG) + 5U));

  size_t total_thermistors = s_afe_settings.num_devices * ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE;
  size_t thermistor_start = s_afe_message_index * NUM_AFE_TEMPERATURES_PER_LOG;

  set_AFE_temperature_id(s_afe_message_index);
  set_AFE_temperature_temperature_0((thermistor_start + 0U) < total_thermistors ? adbms_afe_storage->thermistor_voltages[thermistor_start + 0U] : 0U);
  set_AFE_temperature_temperature_1((thermistor_start + 1U) < total_thermistors ? adbms_afe_storage->thermistor_voltages[thermistor_start + 1U] : 0U);
  set_AFE_temperature_temperature_2((thermistor_start + 2U) < total_thermistors ? adbms_afe_storage->thermistor_voltages[thermistor_start + 2U] : 0U);
  set_AFE_temperature_temperature_3((thermistor_start + 3U) < total_thermistors ? adbms_afe_storage->thermistor_voltages[thermistor_start + 3U] : 0U);
  set_AFE_temperature_temperature_4((thermistor_start + 4U) < total_thermistors ? adbms_afe_storage->thermistor_voltages[thermistor_start + 4U] : 0U);
  set_AFE_temperature_temperature_5((thermistor_start + 5U) < total_thermistors ? adbms_afe_storage->thermistor_voltages[thermistor_start + 5U] : 0U);
  set_AFE_temperature_temperature_6((thermistor_start + 6U) < total_thermistors ? adbms_afe_storage->thermistor_voltages[thermistor_start + 6U] : 0U);

  s_afe_message_index = (s_afe_message_index + 1U) % NUM_AFE_MSGS;

  if (s_afe_message_index == 0U) {
    s_cell_data_updated = false;
  }

  return STATUS_CODE_OK;
}

TASK(cell_sense_conversions, TASK_STACK_512) {
  StatusCode status = STATUS_CODE_OK;
  RETRY_OPERATION(AFE_NUM_RETRIES, RETRY_DELAY_MS, adbms_afe_init(adbms_afe_storage, &s_afe_settings), status);

  delay_ms(10);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (true) {
    s_disable_balancing(); /* We balance again in s_cell_sense_run, disbaled since readings are off during balancing */
    s_cell_sense_conversions();
    s_cell_sense_run();
    xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5000U));
  }
}

StatusCode cell_sense_init(RearControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;
  adbms_afe_storage = &(rear_controller_storage->adbms_afe_storage);
  tasks_init_task(cell_sense_conversions, TASK_PRIORITY(2), NULL);
  return STATUS_CODE_OK;
}
