/************************************************************************************************
 * @file    current_sense.c
 *
 * @brief   Motor precharge handler source file
 *
 * @date    2025-09-02
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "current_sense.h"

#include "global_enums.h"
#include "status.h"

/* Intra-component Headers */
#include "bps_fault.h"
#include "delay.h"
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"
#include "rear_controller_safety_limits.h"
#include "rear_controller_setters.h"
#include "rear_controller_state_manager.h"
#include "tasks.h"

/************************************************************************************************
 * Private defines
 ************************************************************************************************/

/** @brief  Enable latching BPS faults raised by the current-sense task */
#define CSENSE_FAULTS_ENABLED 1U

/** @brief  ADC reference voltage [V] */
#define CSENSE_VREF_V 2.5f

/** @brief  ADS122 PGA gain and its GAIN-field code (gain = 8) */
#define CSENSE_PGA_GAIN 8.0f
#define CSENSE_GAIN_CFG_CODE 0x04U

/** @brief  ADC full-scale range [V] = Vref / gain */
#define CSENSE_FSR_V (CSENSE_VREF_V / CSENSE_PGA_GAIN)

/** @brief  Current-sense shunt resistance [ohm] */
#define CSENSE_SHUNT_RESISTANCE_OHM 0.001f

/** @brief  Empirical calibration: ADC input path under-reads the true shunt voltage; I = V/R * factor */
#define CSENSE_CURRENT_CAL_FACTOR 2.195f

/** @brief  Light EMA smoothing on pack current: y = a*x + (1-a)*y_prev */
#define CSENSE_EMA_ALPHA 0.7f

/** @brief  MUX select for the shunt (current) channel: AIN6 (+) / GND (-) */
#define CSENSE_MUX_SHUNT 0x68U

/** @brief  DEVICE_CFG: CONV_MODE = 1 -> single-shot; START each sample and poll DRDY (mirrors the working smoke test) */
#define CSENSE_DEVICE_CFG_SINGLE_SHOT ADS122_REG_DEVICE_CFG_DEFAULT

/** @brief  REFERENCE_CFG override: Vref = 2.5 V, giving a +-5 V full-scale range */
#define CSENSE_REFERENCE_CFG_VREF_2V5 0x04U

/** @brief  DIGITAL_CFG override: prepend the status byte (STATUS_EN) to each conversion frame */
#define CSENSE_DIGITAL_CFG_STATUS_EN 0x10U

/** @brief  DRDY flag within the status byte (byte 0 of the conversion frame) */
#define CSENSE_DATA_READY_MASK 0x01U

/** @brief  Length of the ADS122 status + 24-bit conversion frame [bytes] */
#define CSENSE_CONVERSION_FRAME_LEN 5U

/** @brief  Shift used to sign-extend the 24-bit conversion result into an int32_t */
#define CSENSE_CONVERSION_SIGN_SHIFT 8U

/** @brief  Full-scale count of the 24-bit signed ADC (2^23) */
#define CSENSE_CONVERSION_FULL_SCALE (1 << 23)

/** @brief  Current-sense sampling period [ms] */
#define CSENSE_SAMPLE_PERIOD_MS 500U

/** @brief  Delay between ADC bring-up retries when the initial config fails [ms] */
#define CSENSE_CONFIG_RETRY_MS 50U

/** @brief  Timeout / step while polling DRDY for a single-shot conversion to complete [ms] */
#define CSENSE_DRDY_POLL_TIMEOUT_MS 100U
#define CSENSE_DRDY_POLL_STEP_MS 2U

static int32_t csense_overcurrents;
static int32_t csense_overvoltages;

#if (IS_USING_CURRENT_SENSE_REV_3 != 0U)

static RearControllerStorage *rear_controller_storage;
static CurrentSenseConfigs current_sense_configs = { .fsr = CSENSE_FSR_V, .mux_config_shunt = CSENSE_MUX_SHUNT, .shunt_resistance_ohm = CSENSE_SHUNT_RESISTANCE_OHM };

/**
 * @brief   Consecutive-event counters for current-sense fault debouncing.
 * @details Mirrors the cell-sense counter pattern: each counter grows while its condition holds and
 *          clears on the first good sample, and the fault latches once the streak crosses threshold.
 */
typedef struct {
  uint32_t comms_retries; /**< consecutive ADS122 I2C transaction failures */
  uint32_t overcurrents;  /**< consecutive over-limit pack-current samples */
} CsenseFaultCounters;

static CsenseFaultCounters s_csense_counters = { 0U };

static float s_csense_ema_A;      /**< EMA-filtered pack current [A] */
static bool s_csense_ema_seeded;  /**< false until the EMA is seeded with the first good sample */

static uint8_t register_map[] = { CSENSE_DEVICE_CFG_SINGLE_SHOT,
                                  ADS122_REG_DATA_RATE_CFG_DEFAULT,
                                  (ADS122_REG_MUX_CFG_DEFAULT | CSENSE_MUX_SHUNT),                     // shunt (current) channel, fixed
                                  (ADS122_REG_GAIN_CFG_DEFAULT | CSENSE_GAIN_CFG_CODE),                 // PGA gain (see CSENSE_PGA_GAIN)
                                  (ADS122_REG_REFERENCE_CFG_DEFAULT | CSENSE_REFERENCE_CFG_VREF_2V5),  // Vref = 2.5 V -> +-5 V range, 256 kHz clock
                                  (ADS122_REG_DIGITAL_CFG_DEFAULT | CSENSE_DIGITAL_CFG_STATUS_EN),
                                  ADS122_REG_GPIO_CFG_DEFAULT,
                                  ADS122_REG_GPIO_DATA_OUTPUT_DEFAULT,
                                  ADS122_REG_IDAC_MAG_CFG_DEFAULT,
                                  ADS122_REG_IDAC_MUX_CFG_DEFAULT,
                                  ADS122_REG_REG_MAP_CRC_DEFAULT };

static inline StatusCode csense_handle_retries(uint32_t *retries, StatusCode status) {
  if (status != STATUS_CODE_OK) {
    *retries += 1;
    if (*retries > REAR_CONTROLLER_CURRENT_SENSE_MAX_RETRIES) {
#if (CSENSE_FAULTS_ENABLED == 1)
      // trigger_bps_fault(BPS_FAULT_COMMS_LOSS_CURR_SENSE);
#endif
    }

    return STATUS_CODE_INTERNAL_ERROR;
  }

  *retries = 0U;

  return STATUS_CODE_OK;
}

TASK(current_sense, TASK_STACK_512) {
  TickType_t xLastWakeTime = xTaskGetTickCount();

  I2CSettings i2c_settings = { .speed = I2C_SPEED_FAST, .sda = GPIO_REAR_CONTROLLER_CURRENT_SENSE_I2C_SDA_GPIO, .scl = GPIO_REAR_CONTROLLER_CURRENT_SENSE_I2C_SCL_GPIO };

  StatusCode status = ads122_init(&rear_controller_storage->ads122_storage, REAR_CONTROLLER_CURRENT_SENSE_I2C_PORT, REAR_CONTOLLER_CURRENT_SENSE_ADC122_I2C_ADDR, register_map, &i2c_settings);
  while (status != STATUS_CODE_OK) {
    delay_ms(CSENSE_CONFIG_RETRY_MS);
    status = ads122_configure(&rear_controller_storage->ads122_storage, register_map);
  }

  while (true) {
    current_sense_run();
    xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CSENSE_SAMPLE_PERIOD_MS));
  }
}

StatusCode current_sense_init(RearControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;

  tasks_init_task(current_sense, TASK_PRIORITY(2), NULL);

  return STATUS_CODE_OK;
}

StatusCode current_sense_run() {
  uint8_t conversion_data_raw[CSENSE_CONVERSION_FRAME_LEN];

  StatusCode status = ads122_start_conversion(&rear_controller_storage->ads122_storage);
  status_ok_or_return(csense_handle_retries(&s_csense_counters.comms_retries, status));

  bool data_ready = false;
  for (uint32_t elapsed = 0U; elapsed < CSENSE_DRDY_POLL_TIMEOUT_MS; elapsed += CSENSE_DRDY_POLL_STEP_MS) {
    status = ads122_get_conversion_data(&rear_controller_storage->ads122_storage, conversion_data_raw);
    status_ok_or_return(csense_handle_retries(&s_csense_counters.comms_retries, status));

    if (conversion_data_raw[0] & CSENSE_DATA_READY_MASK) {
      data_ready = true;
      break;
    }
    delay_ms(CSENSE_DRDY_POLL_STEP_MS);
  }

  if (!data_ready) {
    return STATUS_CODE_TIMEOUT;
  }

  uint32_t conversion_data = ((uint32_t)conversion_data_raw[2] << 16) | ((uint32_t)conversion_data_raw[3] << 8) | ((uint32_t)conversion_data_raw[4]);
  int32_t conversion_data_signed = (int32_t)(conversion_data << CSENSE_CONVERSION_SIGN_SHIFT) >> CSENSE_CONVERSION_SIGN_SHIFT;

  float voltage_V = (float)(conversion_data_signed * current_sense_configs.fsr) / (float)CSENSE_CONVERSION_FULL_SCALE;
  float current_A = voltage_V / ((float)current_sense_configs.shunt_resistance_ohm) * CSENSE_CURRENT_CAL_FACTOR;

  /* Light EMA to tame shunt/ADC noise; seeded on the first good sample */
  if (!s_csense_ema_seeded) {
    s_csense_ema_A = current_A;
    s_csense_ema_seeded = true;
  } else {
    s_csense_ema_A = CSENSE_EMA_ALPHA * current_A + (1.0f - CSENSE_EMA_ALPHA) * s_csense_ema_A;
  }
  current_A = s_csense_ema_A;

  rear_controller_storage->pack_current = current_A;
  set_battery_stats_B_pack_current_a(rear_controller_storage->pack_current);

  /* Debounce: require several consecutive over-limit current samples before latching a fault */
  if (current_A < PACK_MAX_DISCHARGE_CURRENT_A || current_A > PACK_MAX_CHARGE_CURRENT_A) {
    s_csense_counters.overcurrents++;
    if (s_csense_counters.overcurrents > OVERCURRENT_RESPONSE_LOOPS) {
#if (CSENSE_FAULTS_ENABLED == 1)
      BpsFaultData oc_data = { .current = { .current_a = current_A } };
      trigger_bps_fault_with_data(BPS_FAULT_OVERCURRENT, 0U, oc_data);
#endif
    }
  } else {
    s_csense_counters.overcurrents = 0U;
  }

  return STATUS_CODE_OK;
}

#else
static float csense_prev_current_A;
static float csense_prev_voltage_mV;
static int32_t csense_retries;

static RearControllerStorage *rear_controller_storage;

// https://blog.mbedded.ninja/programming/signal-processing/digital-filters/exponential-moving-average-ema-filter/.
float filter_step(const float alpha, float x, float prev_y) {
  return alpha * x + (1 - alpha) * prev_y;
}

StatusCode current_sense_run() {
  float current_reading_A;
  float voltage_reading_mV = 0.0f;

  /* Check current */
  StatusCode status = acs37800_get_current(&rear_controller_storage->acs37800_storage, &current_reading_A);

  if (status != STATUS_CODE_OK) {
    if (csense_retries < REAR_CONTROLLER_CURRENT_SENSE_MAX_RETRIES) {
      csense_retries++;
      return STATUS_CODE_OK;
    } else {
#if (CSENSE_FAULTS_ENABLED == 1)
      trigger_bps_fault(BPS_FAULT_COMMS_LOSS_CURR_SENSE);
#endif
      return STATUS_CODE_OK;
    }
  }

  csense_retries = 0;

  float current_A = filter_step(REAR_CONTROLLER_CURRENT_SENSE_FILTER_ALPHA, current_reading_A, csense_prev_current_A);

  if (current_A < PACK_MAX_DISCHARGE_CURRENT_A || current_A > PACK_MAX_CHARGE_CURRENT_A) {
    csense_overcurrents++;
    if (csense_overcurrents > OVERCURRENT_RESPONSE_LOOPS) {
#if (CSENSE_FAULTS_ENABLED == 1)
      trigger_bps_fault(BPS_FAULT_OVERCURRENT);
#endif
    }
  } else {
    csense_overcurrents = 0;
  }

  /* Check voltage */
  status = acs37800_get_voltage(&rear_controller_storage->acs37800_storage, &voltage_reading_mV);

  if (status != STATUS_CODE_OK) {
    if (csense_retries < REAR_CONTROLLER_CURRENT_SENSE_MAX_RETRIES) {
      csense_retries++;
      return STATUS_CODE_OK;
    } else {
#if (CSENSE_FAULTS_ENABLED == 1)
      trigger_bps_fault(BPS_FAULT_COMMS_LOSS_CURR_SENSE);
#endif
      return STATUS_CODE_OK;
    }
  }

  csense_retries = 0;

  float voltage_mV = filter_step(REAR_CONTROLLER_CURRENT_SENSE_FILTER_ALPHA, voltage_reading_mV, csense_prev_voltage_mV);

  if (voltage_mV > PACK_OVERVOLTAGE_LIMIT_mV) {
    csense_overvoltages++;
    if (csense_overvoltages > OVERCURRENT_RESPONSE_LOOPS) {
#if (CSENSE_FAULTS_ENABLED == 1)
      trigger_bps_fault(BPS_FAULT_OVERVOLTAGE);
#endif
    }
  } else {
    csense_overvoltages = 0;
  }

  /* Store current and voltage in A and V respectively */
  rear_controller_storage->pack_current = (current_A);
  rear_controller_storage->pack_voltage = (voltage_reading_mV / 1000.0f);

  set_battery_stats_B_pack_current_a(rear_controller_storage->pack_current);
  set_battery_stats_A_pack_voltage_v(rear_controller_storage->pack_voltage);

  csense_prev_current_A = current_A;
  csense_prev_voltage_mV = voltage_mV;

  return STATUS_CODE_OK;
}

StatusCode current_sense_init(RearControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;

  status_ok_or_return(acs37800_init(&rear_controller_storage->acs37800_storage, REAR_CONTROLLER_CURRENT_SENSE_I2C_PORT, REAR_CONTROLLER_CURRENT_SENSE_ACS37800_I2C_ADDR));

  return STATUS_CODE_OK;
}
#endif
