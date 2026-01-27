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

#include "current_acs37800.h"
#include "global_enums.h"
#include "status.h"

/* Intra-component Headers */
#include "bps_fault.h"
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"
#include "rear_controller_safety_limits.h"
#include "rear_controller_setters.h"
#include "rear_controller_state_manager.h"

static float csense_prev_current_A;
static float csense_prev_voltage_mV;
static int32_t csense_overcurrents;
static int32_t csense_overvoltages;
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
      trigger_bps_fault(BPS_FAULT_COMMS_LOSS_CURR_SENSE);
      return STATUS_CODE_OK;
    }
  }

  csense_retries = 0;

  float current_A = filter_step(REAR_CONTROLLER_CURRENT_SENSE_FILTER_ALPHA, current_reading_A, csense_prev_current_A);

  if (current_A < PACK_MAX_DISCHARGE_CURRENT_A || current_A > PACK_MAX_CHARGE_CURRENT_A) {
    csense_overcurrents++;
    if (csense_overcurrents > OVERCURRENT_RESPONSE_LOOPS) {
      trigger_bps_fault(BPS_FAULT_OVERCURRENT);
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
      trigger_bps_fault(BPS_FAULT_COMMS_LOSS_CURR_SENSE);
      return STATUS_CODE_OK;
    }
  }

  csense_retries = 0;

  float voltage_mV = filter_step(REAR_CONTROLLER_CURRENT_SENSE_FILTER_ALPHA, voltage_reading_mV, csense_prev_voltage_mV);

  if (voltage_mV > PACK_OVERVOLTAGE_LIMIT_mV) {
    csense_overvoltages++;
    if (csense_overvoltages > OVERCURRENT_RESPONSE_LOOPS) {
      trigger_bps_fault(BPS_FAULT_OVERVOLTAGE);
    }
  } else {
    csense_overvoltages = 0;
  }

  /* Store current and voltage in mA and mV respectively */
  rear_controller_storage->pack_current = (int32_t)(current_A * 1000.0f);
  rear_controller_storage->pack_voltage = (uint32_t)(voltage_reading_mV);

  set_battery_stats_A_pack_current(rear_controller_storage->pack_current);
  set_battery_stats_A_pack_voltage(rear_controller_storage->pack_voltage);

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
