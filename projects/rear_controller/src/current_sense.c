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
#include "status.h"

/* Intra-component Headers */
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"
#include "rear_controller_safety_limits.h"
#include "rear_controller_state_manager.h"

static float csense_prev_current;
static int32_t csense_overcurrents;
static int32_t csense_retries;

static RearControllerStorage *rear_controller_storage;

// https://blog.mbedded.ninja/programming/signal-processing/digital-filters/exponential-moving-average-ema-filter/.
float filter_step(const float alpha, float x, float prev_y) {
  return alpha * x + (1 - alpha) * prev_y;
}

StatusCode current_sense_run() {
  float current_reading;

  StatusCode status = acs37800_get_current(&rear_controller_storage->acs37800_storage, &current_reading);

  if (status != STATUS_CODE_OK) {
    if (csense_retries < REAR_CONTROLLER_CURRENT_SENSE_MAX_RETRIES) {
      csense_retries++;
      return STATUS_CODE_OK;
    } else {
      rear_controller_state_manager_step(REAR_CONTROLLER_STATE_FAULT);
      return STATUS_CODE_OK;
    }
  }

  csense_retries = 0;

  float current = filter_step(REAR_CONTROLLER_CURRENT_SENSE_FILTER_ALPHA, current_reading, csense_prev_current);

  if (current > PACK_MAX_DISCHARGE_CURRENT_A || current < PACK_MAX_CHARGE_CURRENT_A) {
    csense_overcurrents++;
    if (csense_overcurrents > OVERCURRENT_RESPONSE_LOOPS) {
      rear_controller_state_manager_step(REAR_CONTROLLER_STATE_FAULT);
    }
  } else {
    csense_overcurrents = 0;
  }

  csense_prev_current = current;

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
