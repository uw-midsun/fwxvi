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
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"
#include "rear_controller_safety_limits.h"
#include "rear_controller_state_manager.h"

/* Intra-component Headers */
#include "status.h"

static RearControllerStorage *rear_controller_state;

// https://blog.mbedded.ninja/programming/signal-processing/digital-filters/exponential-moving-average-ema-filter/.
float filter_step(const float alpha, float x, float prev_y) {
  return alpha * x + (1 - alpha) * prev_y;
}

StatusCode current_sense_run() {
  float current_reading;

  StatusCode status = acs37800_get_current(&rear_controller_state->acs37800, &current_reading);
  if (status != STATUS_CODE_OK) {
    if (rear_controller_state->csense_retries < REAR_CONTROLLER_CURRENT_SENSE_MAX_RETRIES) {
      rear_controller_state->csense_retries++;
      return STATUS_CODE_OK;
    } else {
      rear_controller_state_manager_step(REAR_CONTROLLER_STATE_FAULT);
      return STATUS_CODE_OK;
    }
  }
  rear_controller_state->csense_retries = 0;

  float current = filter_step(REAR_CONTROLLER_CURRENT_SENSE_FILTER_ALPHA, current_reading, rear_controller_state->csense_prev_current);

  if (current > PACK_MAX_DISCHARGE_CURRENT_A || current < PACK_MAX_CHARGE_CURRENT_A) {
    rear_controller_state->csense_overcurrents++;
    if (rear_controller_state->csense_overcurrents > OVERCURRENT_RESPONSE_LOOPS) {
      rear_controller_state_manager_step(REAR_CONTROLLER_STATE_FAULT);
    }
  } else {
    rear_controller_state->csense_overcurrents = 0;
  }

  rear_controller_state->csense_prev_current = current;

  return STATUS_CODE_OK;
}

StatusCode current_sense_init(RearControllerStorage *state) {
  if (state == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_state = state;

  ACS37800_Storage storage;
  StatusCode status = acs37800_init(&storage, REAR_CONTROLLER_CURRENT_SENSE_I2C_PORT, REAR_CONTROLLER_CURRENT_SENSE_ACS37800_I2C_ADDR);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  state->acs37800 = storage;

  return STATUS_CODE_OK;
}
