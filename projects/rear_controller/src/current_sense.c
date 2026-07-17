/************************************************************************************************
 * @file    precharge.c
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
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"
#include "rear_controller_safety_limits.h"
#include "rear_controller_setters.h"
#include "rear_controller_state_manager.h"

#define CSENSE_FAULTS_ENABLED 1U

static int32_t csense_overcurrents;
static int32_t csense_overvoltages;

#if (IS_USING_CURRENT_SENSE_REV_3 != 0U)

typedef enum {
  CSENSE_HV_BUS,
  CSENSE_SHUNT,
} CsenseStates;

static RearControllerStorage *rear_controller_storage;
/*FSR = Vref / Gain -> Vref = 2.5, Gain = 0.5, mux_config_shunt -> AIN6 and AIN7, mux_config_hv -> AIN0 and AIN1*/
static CurrentSenseConfigs current_sense_configs = {
  .fsr = 5, .mux_config_shunt = 0x67, .mux_config_hv = 0x01, .shunt_resistance_ohm = 0.0005, .resistance_R6_ohm = 1000000U, .resistance_R7_ohm = 20000U
};
static CsenseStates csense_state = CSENSE_HV_BUS;

/**
 * @brief   Consecutive-event counters for current-sense fault debouncing.
 * @details Mirrors the cell-sense counter pattern: each counter grows while its condition holds and
 *          clears on the first good sample, and the fault latches once the streak crosses threshold.
 */
typedef struct {
  uint32_t comms_retries; /**< consecutive ADS122 I2C transaction failures */
  uint32_t overcurrents;  /**< consecutive over-limit pack-current samples */
  uint32_t overvoltages;  /**< consecutive over-limit pack-voltage samples */
} CsenseFaultCounters;

static CsenseFaultCounters s_csense_counters = { 0U };

static uint8_t register_map[] = { ADS122_REG_DEVICE_CFG_DEFAULT,
                                  ADS122_REG_DATA_RATE_CFG_DEFAULT,
                                  (ADS122_REG_MUX_CFG_DEFAULT | 0x01),        // reads voltage first
                                  ADS122_REG_GAIN_CFG_DEFAULT,                // Gain is 0.5
                                  (ADS122_REG_REFERENCE_CFG_DEFAULT | 0x04),  // Vref = 2.5 V -> max range is +- 5 V, clock speed is 256 kHz
                                  (ADS122_REG_DIGITAL_CFG_DEFAULT | 0x10),
                                  ADS122_REG_GPIO_CFG_DEFAULT,
                                  ADS122_REG_GPIO_DATA_OUTPUT_DEFAULT,
                                  ADS122_REG_IDAC_MAG_CFG_DEFAULT,
                                  ADS122_REG_IDAC_MUX_CFG_DEFAULT,
                                  ADS122_REG_REG_MAP_CRC_DEFAULT };

StatusCode current_sense_init(RearControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;

  I2CSettings i2c_settings = { .speed = I2C_SPEED_FAST, .sda = GPIO_REAR_CONTROLLER_CURRENT_SENSE_I2C_SDA_GPIO, .scl = GPIO_REAR_CONTROLLER_CURRENT_SENSE_I2C_SCL_GPIO };

  status_ok_or_return(ads122_init(&storage->ads122_storage, REAR_CONTROLLER_CURRENT_SENSE_I2C_PORT, REAR_CONTOLLER_CURRENT_SENSE_ADC122_I2C_ADDR, register_map, &i2c_settings));

  status_ok_or_return(ads122_change_MUX(&storage->ads122_storage, current_sense_configs.mux_config_shunt));

  return STATUS_CODE_OK;
}

static inline StatusCode csense_handle_retries(uint32_t *retries, StatusCode status) {
  if (status != STATUS_CODE_OK) {
    *retries += 1;
    if (*retries > REAR_CONTROLLER_CURRENT_SENSE_MAX_RETRIES) {
#if (CSENSE_FAULTS_ENABLED == 1)
      trigger_bps_fault(BPS_FAULT_COMMS_LOSS_CURR_SENSE);
#endif
    }

    return STATUS_CODE_INTERNAL_ERROR;
  }

  *retries = 0U;

  return STATUS_CODE_OK;
}

StatusCode current_sense_run() {
  static bool is_reading = false;
  static bool read_current = true;

  StatusCode status;
  if (is_reading) {
    uint8_t conversion_data_raw[5U];
    status = ads122_get_conversion_data(&rear_controller_storage->ads122_storage, conversion_data_raw);
    status_ok_or_return(csense_handle_retries(&s_csense_counters.comms_retries, status));

    bool data_ready = conversion_data_raw[0] & 0x01;
    if (!data_ready) {
      return STATUS_CODE_OK;
    }

    uint32_t conversion_data = ((uint32_t)conversion_data_raw[2] << 16) | ((uint32_t)conversion_data_raw[3] << 8) | ((uint32_t)conversion_data_raw[4]);
    int32_t conversion_data_signed = (int32_t)(conversion_data << 8) >> 8;

    float voltage_V = (float)(conversion_data_signed * current_sense_configs.fsr) / (float)(1 << 23);

    if (read_current) {
      float current_A = voltage_V / ((float)current_sense_configs.shunt_resistance_ohm);

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
        read_current = false;
      }
    } else {
      float hv_voltage_V = voltage_V * (current_sense_configs.resistance_R6_ohm + current_sense_configs.resistance_R7_ohm) / current_sense_configs.resistance_R7_ohm;

      rear_controller_storage->pack_voltage = hv_voltage_V;
      set_battery_stats_A_pack_voltage_v(rear_controller_storage->pack_voltage);

      if (hv_voltage_V > PACK_OVERVOLTAGE_LIMIT_mV * 0.001) {
        s_csense_counters.overvoltages++;
        if (s_csense_counters.overvoltages > OVERCURRENT_RESPONSE_LOOPS) {
#if (CSENSE_FAULTS_ENABLED == 1)
          trigger_bps_fault(BPS_FAULT_OVERVOLTAGE);
#endif
        }
      } else {
        s_csense_counters.overvoltages = 0U;
        read_current = true;
      }
    }
    is_reading = false;
  } else {
    if (read_current) {
      status = ads122_change_MUX(&rear_controller_storage->ads122_storage, current_sense_configs.mux_config_shunt);
      status_ok_or_return(csense_handle_retries(&s_csense_counters.comms_retries, status));
    } else {
      status = ads122_change_MUX(&rear_controller_storage->ads122_storage, current_sense_configs.mux_config_hv);
      status_ok_or_return(csense_handle_retries(&s_csense_counters.comms_retries, status));
    }

    status = ads122_start_conversion(&rear_controller_storage->ads122_storage);
    status_ok_or_return(csense_handle_retries(&s_csense_counters.comms_retries, status));

    is_reading = true;
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
