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

/** @brief  Number of temperatures transmitted in a single log */
#define NUM_AFE_TEMPERATURES_PER_LOG 7U

/** @brief  Number of temperature messages needed to cover all thermistors */
#define NUM_AFE_TEMPERATURE_MSGS ((ADBMS_AFE_MAX_CELL_THERMISTORS + NUM_AFE_TEMPERATURES_PER_LOG - 1U) / NUM_AFE_TEMPERATURES_PER_LOG)

/** @brief  Number of communication retries before throwing AFE fault */
#define AFE_NUM_RETRIES 3U

/** @brief  Plausible thermistor reading band [C]. A healthy sensor never reads below the low bound,
 *          nor above the high bound (a real hot cell trips the 60 C over-temp limit long before
 *          reaching that ceiling), so a reading outside this band means a shorted, open, or
 *          disconnected thermistor rather than a real temperature. */
#define CELL_TEMP_PLAUSIBLE_LOW_C 10
#define CELL_TEMP_PLAUSIBLE_HIGH_C 80

/** @brief  Consecutive over-limit reads required before a thermistor latches an over-temp fault.
 *          Debounces single noisy samples. cell_sense runs every 5 s, so 2 cycles ~= 10 s to trip. */
#define THERMISTOR_OVERTEMP_DEBOUNCE_CYCLES 2U

/** @brief  Consecutive implausible reads before a thermistor latches a disconnected/broken fault. */
#define THERMISTOR_BROKEN_DEBOUNCE_CYCLES 2U

/** @brief  Consecutive out-of-range reads required before a cell over/under-voltage latches a fault.
 *          Debounces single noisy samples. cell_sense runs every 5 s, so 2 cycles ~= 10 s to trip. */
#define CELL_VOLTAGE_FAULT_DEBOUNCE_CYCLES 1U

/** @brief  Maximum pack current for cell discharging current - 7.0A -> 7000mA */
#define MAX_PACK_CURRENT_FOR_CELL_DISCHARGING 7.0f

#define ADBMS1818_ADC_DC_OFFSET_10UV 500

#define THERMISTOR_TEMP_OFFSET_C 4U

/** @brief  Private define to lookup cell voltage */
#define CELL_PER_DEVICE (ADBMS_AFE_MAX_CELLS_PER_DEVICE)

#define CELL_VOLTAGE_LOOKUP(dev_num, cell) (adbms_afe_storage->cell_voltages[adbms_afe_storage->cell_result_lookup[CELL_PER_DEVICE * dev_num + cell]])

#define SET_AFE_STATUS_MESSAGE(device, message_name, message_id, voltage_a, voltage_b, voltage_c) \
  do {                                                                                            \
    set_##message_name##_id(message_id);                                                          \
    set_##message_name##_voltage_##voltage_a(CELL_VOLTAGE_LOOKUP(device, voltage_a));             \
    set_##message_name##_voltage_##voltage_b(CELL_VOLTAGE_LOOKUP(device, voltage_b));             \
    set_##message_name##_voltage_##voltage_c(CELL_VOLTAGE_LOOKUP(device, voltage_c));             \
  } while (0)

#define AFE_THERMISTOR_TX(offset) (((thermistor_start + (offset)) < total_thermistors) ? (uint8_t)(adbms_afe_storage->thermistor_voltages[thermistor_start + (offset)] >> 8U) : 0U)

#define AFE_TEMPERATURE_TX(offset) (((thermistor_start + (offset)) < total_thermistors) ? (uint8_t)(adbms_afe_storage->thermistor_voltages[thermistor_start + (offset)]) : 0U)

/** @brief  Max number of retries for reading cell*/
#define CELL_SENSE_MAX_RETRIES 5U

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

#define THERMISTORS_CONNECTED 1U
#define BALANCING_ENABLED 1U
#define OVER_UNDER_FAULTS_ENABLED 1U
#define THERMISTOR_FAULTS_ENABLED 0U

#define CELL_SENSE_DEBUG 1U

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

static uint8_t s_afe_temperature_message_index = 0U;

/**
 * @brief   Consecutive-failure retry counters, one per ADBMS AFE transaction stage.
 * @details Each stage tracks its own streak so a comms-loss fault names the exact failing operation
 *          and one flaky stage neither masks nor resets another. A counter is cleared as soon as its
 *          stage succeeds, and latches BPS_FAULT_COMMS_LOSS_AFE once it hits AFE_NUM_RETRIES in a row.
 */
typedef struct {
  uint8_t cell_conv;       /**< adbms_afe_trigger_cell_conv failures */
  uint8_t cell_read;       /**< adbms_afe_read_cells failures */
  uint8_t thermistor_conv; /**< adbms_afe_trigger_thermistor_conv failures */
  uint8_t thermistor_read; /**< adbms_afe_read_thermistors failures */
} AfeRetryCounters;

static AfeRetryCounters s_afe_retries = { 0U };

/**
 * @brief   Consecutive out-of-range read counters used to debounce cell-sense faults.
 * @details A counter grows while its condition holds and clears on the first in-range read; the
 *          fault only latches once the streak reaches its debounce threshold.
 */
typedef struct {
  uint8_t thermistor_overtemp[ADBMS_AFE_MAX_CELL_THERMISTORS]; /**< per-thermistor over-temp streak */
  uint8_t thermistor_broken[ADBMS_AFE_MAX_CELL_THERMISTORS];   /**< per-thermistor implausible-read streak */
  uint8_t overvoltage;                                         /**< pack over-voltage streak */
  uint8_t undervoltage;                                        /**< pack under-voltage streak */
} CellFaultDebounce;

static CellFaultDebounce s_fault_debounce = { 0 };

static RearControllerStorage *rear_controller_storage;

static uint8_t s_global_cell_index_1_based(uint8_t device, uint8_t cell) {
  return (uint8_t)(device * ADBMS_AFE_MAX_CELLS_PER_DEVICE + cell + 1U);
}

static uint8_t s_global_thermistor_index_1_based(uint8_t device, uint8_t thermistor) {
  return (uint8_t)(device * ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE + thermistor + 1U);
}

/* The art of programming. */
static void s_set_afe_discharge_status_message(uint8_t dev_index_1_based, uint8_t cell_index, uint8_t is_enabled) {
  switch (dev_index_1_based) {
    case 1U:
      switch (cell_index) {
        case 0U:
          set_AFE_discharge_bitset_AFE1_cell_0(is_enabled);
          break;
        case 1U:
          set_AFE_discharge_bitset_AFE1_cell_1(is_enabled);
          break;
        case 2U:
          set_AFE_discharge_bitset_AFE1_cell_2(is_enabled);
          break;
        case 3U:
          set_AFE_discharge_bitset_AFE1_cell_3(is_enabled);
          break;
        case 4U:
          set_AFE_discharge_bitset_AFE1_cell_4(is_enabled);
          break;
        case 5U:
          set_AFE_discharge_bitset_AFE1_cell_5(is_enabled);
          break;
        case 6U:
          set_AFE_discharge_bitset_AFE1_cell_6(is_enabled);
          break;
        case 7U:
          set_AFE_discharge_bitset_AFE1_cell_7(is_enabled);
          break;
        case 8U:
          set_AFE_discharge_bitset_AFE1_cell_8(is_enabled);
          break;
        case 9U:
          set_AFE_discharge_bitset_AFE1_cell_9(is_enabled);
          break;
        case 10U:
          set_AFE_discharge_bitset_AFE1_cell_10(is_enabled);
          break;
        case 11U:
          set_AFE_discharge_bitset_AFE1_cell_11(is_enabled);
          break;
        case 12U:
          set_AFE_discharge_bitset_AFE1_cell_12(is_enabled);
          break;
        case 13U:
          set_AFE_discharge_bitset_AFE1_cell_13(is_enabled);
          break;
        case 14U:
          set_AFE_discharge_bitset_AFE1_cell_14(is_enabled);
          break;
        case 15U:
          set_AFE_discharge_bitset_AFE1_cell_15(is_enabled);
          break;
        default:
          break;
      }
      break;

    case 2U:
      switch (cell_index) {
        case 0U:
          set_AFE_discharge_bitset_AFE2_cell_0(is_enabled);
          break;
        case 1U:
          set_AFE_discharge_bitset_AFE2_cell_1(is_enabled);
          break;
        case 2U:
          set_AFE_discharge_bitset_AFE2_cell_2(is_enabled);
          break;
        case 3U:
          set_AFE_discharge_bitset_AFE2_cell_3(is_enabled);
          break;
        case 4U:
          set_AFE_discharge_bitset_AFE2_cell_4(is_enabled);
          break;
        case 5U:
          set_AFE_discharge_bitset_AFE2_cell_5(is_enabled);
          break;
        case 6U:
          set_AFE_discharge_bitset_AFE2_cell_6(is_enabled);
          break;
        case 7U:
          set_AFE_discharge_bitset_AFE2_cell_7(is_enabled);
          break;
        case 8U:
          set_AFE_discharge_bitset_AFE2_cell_8(is_enabled);
          break;
        case 9U:
          set_AFE_discharge_bitset_AFE2_cell_9(is_enabled);
          break;
        case 10U:
          set_AFE_discharge_bitset_AFE2_cell_10(is_enabled);
          break;
        case 11U:
          set_AFE_discharge_bitset_AFE2_cell_11(is_enabled);
          break;
        case 12U:
          set_AFE_discharge_bitset_AFE2_cell_12(is_enabled);
          break;
        case 13U:
          set_AFE_discharge_bitset_AFE2_cell_13(is_enabled);
          break;
        case 14U:
          set_AFE_discharge_bitset_AFE2_cell_14(is_enabled);
          break;
        case 15U:
          set_AFE_discharge_bitset_AFE2_cell_15(is_enabled);
          break;
        default:
          break;
      }
      break;

    default:
      break;
  }
}

/************************************************************************************************
 * Private function definitions
 ************************************************************************************************/

static void s_balance_cells(uint16_t min_voltage) {
#if (BALANCING_ENABLED == 1U)
  uint16_t balancing_threshold = min_voltage;

  if (!get_steering_buttons_balancing_enabled()) {
    return;
  }

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
#if (CELL_SENSE_DEBUG == 1)
        LOG_DEBUG("DISCHRG CELL %d %d\r\n", (uint8_t)dev, (uint8_t)cell);
        delay_ms(12U);
#endif

        adbms_afe_toggle_cell_discharge(adbms_afe_storage, global_cell, true);
        s_set_afe_discharge_status_message((dev + 1), cell, 1U);

      } else {
        adbms_afe_toggle_cell_discharge(adbms_afe_storage, global_cell, false);
        s_set_afe_discharge_status_message((dev + 1), cell, 0U);
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
      s_set_afe_discharge_status_message((dev + 1U), cell, 0U);
    }
  }

  /* Commit the discharge configuration to the ADBMS1818 */
  adbms_afe_write_config(adbms_afe_storage);
}

static StatusCode s_check_thermistors() {
  StatusCode status = STATUS_CODE_OK;
#if (THERMISTORS_CONNECTED == 1U)
  /* Convert each raw thermistor reading to a temperature in-place so the AFE temperature message transmits degrees C */
  for (uint8_t device = 0U; device < s_afe_settings.num_devices; device++) {
    for (uint8_t thermistor = 0U; thermistor < ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE; thermistor++) {
      uint8_t index = device * ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE + thermistor;
      adbms_afe_storage->thermistor_voltages[index] = calculate_board_thermistor_temperature(adbms_afe_storage->thermistor_voltages[index] / 10U) - THERMISTOR_TEMP_OFFSET_C;

#if (THERMISTOR_FAULTS_ENABLED == 1U)
      uint16_t temperature_c = adbms_afe_storage->thermistor_voltages[index];

      /* Ignore implausible temperature readings */
      if (temperature_c < CELL_TEMP_PLAUSIBLE_LOW_C || temperature_c > CELL_TEMP_PLAUSIBLE_HIGH_C) {
        continue;
      }
      /* A valid in-range read clears the broken-sensor debounce for this thermistor */
      s_fault_debounce.thermistor_broken[index] = 0U;

      /* Discharging (current < 0) uses the discharge limit, otherwise the charge limit */
      uint16_t overtemp_limit = (rear_controller_storage->pack_current < 0) ? CELL_OVERTEMP_DISCHARGE_LIMIT_C : CELL_OVERTEMP_CHARGE_LIMIT_C;

      if (temperature_c >= overtemp_limit) {
        /* Debounce: only latch after several consecutive over-limit reads to reject a single noisy sample */
        if (++s_fault_debounce.thermistor_overtemp[index] >= THERMISTOR_OVERTEMP_DEBOUNCE_CYCLES) {
          // LOG_DEBUG("CELL OVERTEMP\n");
          uint8_t cell = s_global_thermistor_index_1_based(device, thermistor);
          BpsFaultData data = { .temp = { .cell_index = cell, .temperature_c = (int16_t)temperature_c } };
          trigger_bps_fault_with_data(BPS_FAULT_OVERTEMP_CELL, cell, data);
          status = STATUS_CODE_INTERNAL_ERROR;
        }
      } else {
        /* A valid in-range read clears the debounce for this thermistor */
        s_fault_debounce.thermistor_overtemp[index] = 0U;
      }
#endif
    }
  }
#endif
  return status;
}

/**
 * @brief   Record the result of one AFE transaction stage against its own retry counter.
 * @details On success the streak is cleared. On failure the streak grows, and once it reaches
 *          AFE_NUM_RETRIES in a row a comms-loss fault is latched (when fault_enabled).
 * @param   status        Result returned by the AFE transaction
 * @param   retry_count   Pointer to this stage's counter in s_afe_retries
 * @param   fault_enabled Whether a sustained failure of this stage should latch a BPS fault
 * @param   stage_name    Human-readable stage name for the debug log
 * @return  true if the stage succeeded, false if it failed (caller should abort the cycle)
 */
static bool s_afe_stage_ok(StatusCode status, uint8_t *retry_count, bool fault_enabled, const char *stage_name) {
  if (status == STATUS_CODE_OK) {
    *retry_count = 0U;
    return true;
  }

  (*retry_count)++;
  LOG_DEBUG("AFE %s failed: status %d (retry %u)\n", stage_name, status, *retry_count);

  if (fault_enabled && *retry_count >= AFE_NUM_RETRIES) {
    trigger_bps_fault(BPS_FAULT_COMMS_LOSS_AFE);
  }

  return false;
}

static StatusCode s_cell_sense_conversions() {
  StatusCode status;

  status = adbms_afe_trigger_cell_conv(adbms_afe_storage);
  if (!s_afe_stage_ok(status, &s_afe_retries.cell_conv, OVER_UNDER_FAULTS_ENABLED, "cell conv")) {
    return status;
  }

  delay_ms(CONV_DELAY_MS);

  status = adbms_afe_read_cells(adbms_afe_storage);
  if (!s_afe_stage_ok(status, &s_afe_retries.cell_read, OVER_UNDER_FAULTS_ENABLED, "cell read")) {
    return status;
  }

#if (THERMISTORS_CONNECTED == 1U)
  status = adbms_afe_trigger_thermistor_conv(adbms_afe_storage);
  if (!s_afe_stage_ok(status, &s_afe_retries.thermistor_conv, THERMISTOR_FAULTS_ENABLED, "thermistor conv")) {
    return status;
  }

  delay_ms(AUX_CONV_DELAY_MS);

  status = adbms_afe_read_thermistors(adbms_afe_storage);
  if (!s_afe_stage_ok(status, &s_afe_retries.thermistor_read, THERMISTOR_FAULTS_ENABLED, "thermistor read")) {
    return status;
  }
#endif

  return STATUS_CODE_OK;
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
      CELL_VOLTAGE_LOOKUP(dev, cell) += ADBMS1818_ADC_DC_OFFSET_10UV;
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

  rear_controller_storage->pack_voltage = total_voltage / 10000.0f;
  set_battery_stats_A_pack_voltage_v(rear_controller_storage->pack_voltage);

  CONDITIONAL_LOG_DEBUG("PACK V: %d\r\n", (int)rear_controller_storage->pack_voltage);
  delay_ms(10U);
  CONDITIONAL_LOG_DEBUG("MAX VOLTAGE: %d\r\nMIN VOLTAGE: %d\r\nUNBALANCE: %d\r\n", max_voltage, min_voltage, max_voltage - min_voltage);
  delay_ms(10U);

  set_battery_stats_B_max_cell_voltage(max_voltage);
  set_battery_stats_B_min_cell_voltage(min_voltage);

  /* BPS disabled from steering is a manual override: leave solar connected so it does not fight the RESET re-close */
  if (max_voltage >= SOLAR_VOLTAGE_THRESHOLD && !bps_is_disabled()) {
    relays_open_solar();
  }

  /*
   * Compare max and min voltages to safety limits
   * We must multiply the safety limit of 10 to convert from mV -> 100 uV
   */
  /* Debounce over/under-voltage: only latch after several consecutive out-of-range reads to reject a single noisy sample */
  if (max_voltage >= (CELL_OVERVOLTAGE_LIMIT_mV * 10U)) {
    if (++s_fault_debounce.overvoltage >= CELL_VOLTAGE_FAULT_DEBOUNCE_CYCLES) {
      LOG_DEBUG("FAULT: OVERVOLTAGE: %u\r\n", max_voltage);
#if (OVER_UNDER_FAULTS_ENABLED == 1)
      BpsFaultData ov_data = { .cell = { .cell_index = (uint8_t)max_voltage_cell, .cell_voltage = max_voltage } };
      trigger_bps_fault_with_data(BPS_FAULT_OVERVOLTAGE, (uint8_t)max_voltage_cell, ov_data);
#endif
      status = STATUS_CODE_INTERNAL_ERROR;
    }
  } else {
    s_fault_debounce.overvoltage = 0U;
  }

  if (min_voltage <= (CELL_UNDERVOLTAGE_LIMIT_mV * 10U)) {
    if (++s_fault_debounce.undervoltage >= CELL_VOLTAGE_FAULT_DEBOUNCE_CYCLES) {
      LOG_DEBUG("FAULT: UNDERVOLTAGE: %u\r\n", min_voltage);
#if (OVER_UNDER_FAULTS_ENABLED == 1)
      BpsFaultData uv_data = { .cell = { .cell_index = (uint8_t)min_voltage_cell, .cell_voltage = min_voltage } };
      trigger_bps_fault_with_data(BPS_FAULT_UNDERVOLTAGE, (uint8_t)min_voltage_cell, uv_data);
#endif
      status = STATUS_CODE_INTERNAL_ERROR;
    }
  } else {
    s_fault_debounce.undervoltage = 0U;
  }

  if ((max_voltage - min_voltage) >= (CELL_UNBALANCED_LIMIT_mV * 10)) {
    /* Note (From Aryan): We don't actually need to fault on imbalance. It is here for safety. Remove if needed */
    LOG_DEBUG("FAULT: UNBALANCED: %u\r\n", max_voltage - min_voltage);
#if (OVER_UNDER_FAULTS_ENABLED == 1)
    // BpsFaultData unbal_data = { .unbalance = {
    //                                 .max_cell_index = (uint8_t)max_voltage_cell, .min_cell_index = (uint8_t)min_voltage_cell, .max_cell_voltage = max_voltage, .min_cell_voltage = min_voltage } };
    // trigger_bps_fault_with_data(BPS_FAULT_UNBALANCE, (uint8_t)max_voltage_cell, unbal_data);
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
  if (s_cell_data_updated != true) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  SET_AFE_STATUS_MESSAGE(0U, AFE1_status_A, 0U, 0, 1, 2);
  SET_AFE_STATUS_MESSAGE(0U, AFE1_status_B, 1U, 3, 4, 5);
  SET_AFE_STATUS_MESSAGE(0U, AFE1_status_C, 2U, 6, 7, 8);
  SET_AFE_STATUS_MESSAGE(0U, AFE1_status_D, 3U, 9, 10, 11);
  SET_AFE_STATUS_MESSAGE(0U, AFE1_status_E, 4U, 12, 13, 14);
  SET_AFE_STATUS_MESSAGE(0U, AFE1_status_F, 5U, 15, 16, 17);

  SET_AFE_STATUS_MESSAGE(1U, AFE2_status_A, 0U, 0, 1, 2);
  SET_AFE_STATUS_MESSAGE(1U, AFE2_status_B, 1U, 3, 4, 5);
  SET_AFE_STATUS_MESSAGE(1U, AFE2_status_C, 2U, 6, 7, 8);
  SET_AFE_STATUS_MESSAGE(1U, AFE2_status_D, 3U, 9, 10, 11);
  SET_AFE_STATUS_MESSAGE(1U, AFE2_status_E, 4U, 12, 13, 14);
  SET_AFE_STATUS_MESSAGE(1U, AFE2_status_F, 5U, 15, 16, 17);

  size_t total_thermistors = s_afe_settings.num_devices * ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE;
  size_t thermistor_start = s_afe_temperature_message_index * NUM_AFE_TEMPERATURES_PER_LOG;

  set_AFE_temperature_id(s_afe_temperature_message_index);
  set_AFE_temperature_temperature_0(AFE_TEMPERATURE_TX(0U));
  set_AFE_temperature_temperature_1(AFE_TEMPERATURE_TX(1U));
  set_AFE_temperature_temperature_2(AFE_TEMPERATURE_TX(2U));
  set_AFE_temperature_temperature_3(AFE_TEMPERATURE_TX(3U));
  set_AFE_temperature_temperature_4(AFE_TEMPERATURE_TX(4U));
  set_AFE_temperature_temperature_5(AFE_TEMPERATURE_TX(5U));
  set_AFE_temperature_temperature_6(AFE_TEMPERATURE_TX(6U));

  s_afe_temperature_message_index = (s_afe_temperature_message_index + 1U) % NUM_AFE_TEMPERATURE_MSGS;

  if (s_afe_temperature_message_index == 0U) {
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
