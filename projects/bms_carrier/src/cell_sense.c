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
#include "delay.h"
#include "global_enums.h"
#include "spi.h"

/* Intra-component Headers */
#include "bms_carrier.h"
#include "bms_hw_defs.h"
#include "cell_sense.h"
#include "fault_bps.h"
#include "relays.h"

/************************************************************************************************
 * Private defines
 ************************************************************************************************/

/** @brief  Number of AFE messages */
#define NUM_AFE_MSGS 4U
/** @brief  Number of cell voltage readings per AFE message */
#define READINGS_PER_AFE_MSG 3U
/** @brief  Thermistor resistance value */
#define TEMP_RESISTANCE 10000
/** @brief  Voltage reference for ADBMS1818 ADCs in 100uV */
#define VREF2 30000.0f
/** @brief  Voltage gain of ADBMS1818 ADCs in 100uV / sample */
#define ADC_GAIN (VREF2 / ((1 << 15) - 1))
/** @brief  Thermistor resistance lookup table size */
#define TABLE_SIZE 125U
/** @brief  Number of communication retries before throwing AFE fault */
#define AFE_NUM_RETRIES 5U
/** @brief  Temperature threshold for invalid readings */
#define CELL_TEMP_OUTLIER_THRESHOLD 80
/** @brief  Private define to lookup cell voltage */
#define CELL_VOLTAGE_LOOKUP(cell) (adbms_afe_storage->cell_voltages[adbms_afe_storage->cell_result_lookup[cell]])

/**
 * @brief   Thermistor mapping for the ADBMS1818 hardware schematic
 */
typedef enum ThermistorMap { THERMISTOR_2 = 0, THERMISTOR_1, THERMISTOR_0, THERMISTOR_3, THERMISTOR_4, THERMISTOR_7, THERMISTOR_5, THERMISTOR_6, NUM_THERMISTORS } ThermistorMap;

/************************************************************************************************
 * Private variables
 ************************************************************************************************/

/* Forward definition */

int calculate_temperature(uint16_t thermistor);

static const AdbmsAfeSettings s_afe_settings = {
  .spi_settings = { .sdo = ADBMS_AFE_SPI_SDO_GPIO, .sdi = ADBMS_AFE_SPI_SDI_GPIO, .sclk = ADBMS_AFE_SPI_SCK_GPIO, .cs = ADBMS_AFE_SPI_CS_GPIO, .baudrate = ADBMS_AFE_SPI_BAUDRATE },

  .spi_port = ADBMS_AFE_SPI_PORT,

  .adc_mode = ADBMS_AFE_ADC_MODE_7KHZ,

  .cell_bitset = { 0xFFFU, 0xFFFU, 0xFFFU },
  .aux_bitset = { 0x14U, 0x15U, 0x15U },

  .num_devices = 3U,
  .num_cells = 12U,
  .num_thermistors = NUM_THERMISTORS,
};

static AdbmsAfeStorage *adbms_afe_storage;

static BmsStorage *bms_storage;

static bool s_cell_data_updated = false;

uint8_t afe_message_index = 0U;

static uint8_t s_thermistor_map[NUM_THERMISTORS] = {
  [0] = THERMISTOR_0, [1] = THERMISTOR_1, [2] = THERMISTOR_2, [3] = THERMISTOR_3, [4] = THERMISTOR_4, [5] = THERMISTOR_5, [6] = THERMISTOR_6, [7] = THERMISTOR_7
};

static const uint16_t s_resistance_lookup[TABLE_SIZE] = { 27219U, 26076U, 24988U, 23951U, 22963U, 22021U, 21123U, 20267U, 19450U, 18670U, 17925U, 17214U, 16534U, 15886U, 15266U, 14674U,
                                                          14173U, 13718U, 13256U, 12805U, 12394U, 12081U, 11628U, 11195U, 10780U, 10000U, 9634U,  9283U,  8947U,  8624U,  8314U,  8018U,
                                                          7733U,  7460U,  7199U,  6947U,  6706U,  6475U,  6252U,  6039U,  5834U,  5636U,  5445U,  5262U,  5093U,  4927U,  4763U,  4601U,
                                                          4446U,  4300U,  4161U,  4026U,  3896U,  3771U,  3651U,  3535U,  3423U,  3315U,  3211U,  3111U,  3014U,  2922U,  2833U,  2748U,
                                                          2665U,  2586U,  2509U,  2435U,  2364U,  2294U,  2227U,  2162U,  2101U,  2040U,  1981U,  1925U,  1868U,  1817U,  1765U,  1716U,
                                                          1668U,  1622U,  1577U,  1533U,  1490U,  1449U,  1410U,  1371U,  1334U,  1298U,  1263U,  1229U,  1197U,  1164U,  1134U,  1107U,
                                                          1078U,  1052U,  1025U,  999U,   973U,   949U,   925U,   902U,   880U,   858U,   837U,   816U,   796U,   777U,   758U,   739U,
                                                          721U,   704U,   687U,   671U,   655U,   640U,   625U,   610U,   596U,   582U,   569U,   556U,   543U };

/************************************************************************************************
 * Private function definitions
 ************************************************************************************************/

static void s_balance_cells(uint16_t min_voltage) {
  uint16_t balancing_threshold = min_voltage;

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
  for (size_t cell = 0U; cell < (s_afe_settings.num_devices * s_afe_settings.num_cells); cell++) {
    if (CELL_VOLTAGE_LOOKUP(cell) > balancing_threshold) {
      adbms_afe_toggle_cell_discharge(adbms_afe_storage, cell, true);
    } else {
      adbms_afe_toggle_cell_discharge(adbms_afe_storage, cell, false);
    }
  }

  /* Commit the discharge configuration to the ADBMS1818 */
  adbms_afe_write_config(adbms_afe_storage);
}

static StatusCode s_check_thermistors() {
  StatusCode status = STATUS_CODE_OK;
  uint16_t max_temp = 0U;

  /* Loop over all devices and thermistors */
  for (uint8_t device = 0U; device < s_afe_settings.num_devices; device++) {
    for (uint8_t thermistor = 0U; thermistor < ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE; thermistor++) {
      /* Check if the temperature ADC is enabled for the provided device and thermistor channel */
      if ((s_afe_settings.aux_bitset[device] >> thermistor) & 1U) {
        /* Calculate the ADC voltage index given the device and thermistor number */
        uint8_t index = device * ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE + thermistor;
        adbms_afe_storage->aux_voltages[index] = calculate_temperature(adbms_afe_storage->aux_voltages[index]);

        LOG_DEBUG("Thermistor reading device %d, %d: %d\n", device, thermistor, adbms_afe_storage->aux_voltages[index]);

        /* Ignore temperature readings outside of the valid temperature range */
        if (adbms_afe_storage->aux_voltages[index] > CELL_TEMP_OUTLIER_THRESHOLD) {
          continue;
        }

        if (adbms_afe_storage->aux_voltages[index] > max_temp) {
          max_temp = adbms_afe_storage->aux_voltages[index];
        }

        delay_ms(3);
        if (bms_storage->pack_current < 0) {
          if (adbms_afe_storage->aux_voltages[index] >= CELL_MAX_TEMPERATURE_DISCHARGE) {
            LOG_DEBUG("CELL OVERTEMP\n");
            fault_bps_set(BMS_FAULT_OVERTEMP_CELL);
            status = STATUS_CODE_INTERNAL_ERROR;
          }
        } else {
          if (adbms_afe_storage->aux_voltages[index] >= CELL_MAX_TEMPERATURE_CHARGE) {
            LOG_DEBUG("CELL OVERTEMP\n");
            fault_bps_set(BMS_FAULT_OVERTEMP_CELL);
            status = STATUS_CODE_INTERNAL_ERROR;
          }
        }
      }
    }
  }

  bms_storage->max_temperature = max_temp;

  return status;
}

static StatusCode s_cell_sense_conversions() {
  StatusCode status = STATUS_CODE_OK;
  // TODO: Figure out why cell_conv cannot happen without spi timing out (Most likely RTOS
  // implemntation error) Retry Mechanism
  for (uint8_t retries = AFE_NUM_RETRIES; retries > 0; retries--) {
    status = adbms_afe_trigger_cell_conv(adbms_afe_storage);
    if (status == STATUS_CODE_OK) {
      break;
    }
    LOG_DEBUG("Cell trigger conv failed, retrying %d\n", status);
    delay_ms(RETRY_DELAY_MS);
  }
  if (status) {
    // If this has failed, try once more after a short delay
    LOG_DEBUG("Cell trigger conv failed, retrying): %d\n", status);
    delay_ms(RETRY_DELAY_MS);
    status = adbms_afe_trigger_cell_conv(adbms_afe_storage);
  }
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Cell conv failed): %d\n", status);
    fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
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
    fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
    return status;
  }

  for (size_t thermistor = 0; thermistor < s_afe_settings.num_thermistors; thermistor += 1) {
    // Thermistor indexes are read for each daisy chained dev at the same time
    // Check therm bitset to determine if we need to read any at this index
    bool check_therm = false;
    for (uint8_t dev = 0; dev < s_afe_settings.num_devices; dev++) {
      if ((s_afe_settings.aux_bitset[dev] >> thermistor) & 0x1) {
        check_therm = true;
      }
    }

    if (check_therm) {
      // Trigger and read thermistor value
      for (uint8_t retries = AFE_NUM_RETRIES; retries > 0; retries--) {
        status = adbms_afe_trigger_aux_conv(adbms_afe_storage, s_thermistor_map[thermistor]);
        if (status == STATUS_CODE_OK) {
          break;
        }
        LOG_DEBUG("Aux trigger conv failed, retrying: %d\n", status);
        delay_ms(RETRY_DELAY_MS);
      }
      if (status) {
        LOG_DEBUG("Thermistor conv failed for therm %d: Status %d\n", (uint8_t)thermistor, status);
        fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
        return status;
      }
      delay_ms(AUX_CONV_DELAY_MS);

      for (uint8_t retries = AFE_NUM_RETRIES; retries > 0; retries--) {
        status = adbms_afe_read_aux(adbms_afe_storage, thermistor);
        if (status == STATUS_CODE_OK) {
          break;
        }
        LOG_DEBUG("Thermistor read failed, retrying %d\n", status);
        delay_ms(RETRY_DELAY_MS);
      }
      if (status) {
        LOG_DEBUG("Thermistor read trigger failed for thermistor %d,  %d\n", (uint8_t)thermistor, status);
        fault_bps_set(BMS_FAULT_COMMS_LOSS_AFE);
        return status;
      }
    }
  }
  return status;
}

static StatusCode s_cell_sense_run() {
  StatusCode status = STATUS_CODE_OK;

  uint16_t max_voltage = 0U;
  uint16_t min_voltage = 0xFFFFU;

  for (size_t cell = 0U; cell < (s_afe_settings.num_devices * s_afe_settings.num_cells); cell++) {
    LOG_DEBUG("CELL %d: %d\n\r", (uint8_t)cell, CELL_VOLTAGE_LOOKUP(cell));
    delay_ms(5U);

    if (CELL_VOLTAGE_LOOKUP(cell) > max_voltage) {
      max_voltage = CELL_VOLTAGE_LOOKUP(cell);
    }
    if (CELL_VOLTAGE_LOOKUP(cell) < min_voltage) {
      min_voltage = CELL_VOLTAGE_LOOKUP(cell);
    }
  }
  delay_ms(10);

  LOG_DEBUG("MAX VOLTAGE: %d\n", max_voltage);
  LOG_DEBUG("MIN VOLTAGE: %d\n", min_voltage);
  LOG_DEBUG("UNBALANCE: %d\n", max_voltage - min_voltage);
  set_battery_info_max_cell_v(max_voltage);
  set_battery_info_min_cell_v(min_voltage);

  if (max_voltage >= SOLAR_VOLTAGE_THRESHOLD) {
    bms_open_solar();
  }

  if (max_voltage >= CELL_OVERVOLTAGE) {
    LOG_DEBUG("OVERVOLTAGE\n");
    fault_bps_set(BMS_FAULT_OVERVOLTAGE);
    status = STATUS_CODE_INTERNAL_ERROR;
  }
  if (min_voltage <= CELL_UNDERVOLTAGE) {
    LOG_DEBUG("UNDERVOLTAGE\n");
    fault_bps_set(BMS_FAULT_UNDERVOLTAGE);
    status = STATUS_CODE_INTERNAL_ERROR;
  }
  if (max_voltage - min_voltage >= CELL_UNBALANCED) {
    LOG_DEBUG("UNBALANCED\n");
    fault_bps_set(BMS_FAULT_UNBALANCE);
    status = STATUS_CODE_INTERNAL_ERROR;
  }

  s_balance_cells(min_voltage);

  status_ok_or_return(s_check_thermistors());

  s_cell_data_updated = true;

  return status;
}

/************************************************************************************************
 * Public function definitions
 ************************************************************************************************/

int calculate_temperature(uint16_t thermistor) {
  thermistor = (uint16_t)(thermistor * ADC_GAIN);                                          // 100uV
  uint16_t thermistor_resistance = (thermistor * TEMP_RESISTANCE) / (VREF2 - thermistor);  // Ohms
  delay_ms(10);
  uint16_t min_diff = abs(thermistor_resistance - s_resistance_lookup[0]);

  for (uint8_t i = 1U; i < TABLE_SIZE; ++i) {
    if (abs(thermistor_resistance - s_resistance_lookup[i]) < min_diff) {
      min_diff = abs(thermistor_resistance - s_resistance_lookup[i]);
      thermistor = i;
    }
  }
  return thermistor;
}

StatusCode log_cell_sense() {
  /* AFE messages are split into 3 (For each AFE) */
  /* We send 4 messages total to transmit all cell voltages (3 per msg * 4 times) = 12 total cell voltages */
  if (s_cell_data_updated != true) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  uint8_t read_index = afe_message_index * READINGS_PER_AFE_MSG;
  set_AFE1_status_id(afe_message_index);
  set_AFE1_status_v1(CELL_VOLTAGE_LOOKUP(read_index));
  set_AFE1_status_v2(CELL_VOLTAGE_LOOKUP(read_index + 1));
  set_AFE1_status_v3(CELL_VOLTAGE_LOOKUP(read_index + 2));

  read_index = (uint8_t)s_afe_settings.num_cells + afe_message_index * READINGS_PER_AFE_MSG;
  set_AFE2_status_id(afe_message_index);
  set_AFE2_status_v1(CELL_VOLTAGE_LOOKUP(read_index));
  set_AFE2_status_v2(CELL_VOLTAGE_LOOKUP(read_index + 1));
  set_AFE2_status_v3(CELL_VOLTAGE_LOOKUP(read_index + 2));

  read_index = (uint8_t)s_afe_settings.num_cells * 2 + afe_message_index * READINGS_PER_AFE_MSG;
  set_AFE3_status_id(afe_message_index);
  set_AFE3_status_v1(CELL_VOLTAGE_LOOKUP(read_index));
  set_AFE3_status_v2(CELL_VOLTAGE_LOOKUP(read_index + 1));
  set_AFE3_status_v3(CELL_VOLTAGE_LOOKUP(read_index + 2));

  // Thermistors to send are at index 0, 2, 4 for each device
  if (afe_message_index < NUM_AFE_MSGS - 1) {  // Only 3 thermistors per device, so 4th message will be ignored
    set_AFE1_status_temp(adbms_afe_storage->aux_voltages[afe_message_index * 2]);
    set_AFE2_status_temp(adbms_afe_storage->aux_voltages[ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE + afe_message_index * 2]);
    set_AFE3_status_temp(adbms_afe_storage->aux_voltages[ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE * 2 + afe_message_index * 2]);
  }

  afe_message_index = (afe_message_index + 1) % NUM_AFE_MSGS;

  if (afe_message_index == 0) {
    s_cell_data_updated = false;
  }

  return STATUS_CODE_OK;
}

TASK(cell_sense_conversions, TASK_STACK_512) {
  adbms_afe_init(adbms_afe_storage, &s_afe_settings);
  delay_ms(10);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (true) {
    s_cell_sense_conversions();
    s_cell_sense_run();
    xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5000U));
  }
}

StatusCode cell_sense_init(BmsStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  bms_storage = storage;
  adbms_afe_storage = &(bms_storage->adbms_afe_storage);
  tasks_init_task(cell_sense_conversions, TASK_PRIORITY(2), NULL);
  return STATUS_CODE_OK;
}
