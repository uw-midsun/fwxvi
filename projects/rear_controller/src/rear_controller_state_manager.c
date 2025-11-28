/***********************************************************************************************
 * @file    rear_controller_state_manager.c
 *
 * @brief   Rear Controller state machine manager source file
 *
 * @date    2025-09-14
 * @author  Midnight Sun Team #24 - MSXVI
 ***********************************************************************************************/

/* Standard library Headers */
#include <stdio.h>

/* Inter-component Headers */
#include "log.h"

/* Intra-component Headers */
#include "rear_controller_safety_limits.h"
#include "rear_controller_state_manager.h"
#include "relays.h"

static RearControllerStorage *rear_controller_storage = NULL;
static RearControllerState s_current_state = REAR_CONTROLLER_STATE_IDLE;

/**
 * @brief   Asynchronous event handler
 */
static void rear_controller_state_manager_enter_state(RearControllerState new_state) {
  switch (new_state) {
    case REAR_CONTROLLER_STATE_IDLE:
      relays_disable_ws22_lv();
      relays_open_motor();
      break;

    case REAR_CONTROLLER_STATE_DRIVE:
      if (rear_controller_storage->precharge_complete) {
        relays_enable_ws22_lv();
        relays_close_motor();
      }

      rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_NEUTRAL_REQUEST);
      break;

    case REAR_CONTROLLER_STATE_CHARGE:
      relays_close_motor();
      /* TODO: Make any changes required for charging. Previous car required us to close motor relay */
      break;

    case REAR_CONTROLLER_STATE_FAULT:
      relays_reset();
      /* TODO: Disable everything for safety, open all relays and disable LV motor */
      break;
  }

  s_current_state = new_state;
  LOG_DEBUG("RearController State Manager entered state: %d\r\n", new_state);
}

StatusCode rear_controller_state_manager_init(RearControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;

  StatusCode status = relays_close_pos();
  status = relays_close_solar();
  status = relays_close_neg();

  if (status == STATUS_CODE_OK) {
    rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_IDLE);
  } else {
    rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_FAULT);
  }

  return STATUS_CODE_OK;
}

StatusCode rear_controller_state_manager_step(RearControllerEvent event) {
  if (rear_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  switch (s_current_state) {
    case REAR_CONTROLLER_STATE_IDLE:
      if (event == REAR_CONTROLLER_EVENT_DRIVE_REQUEST)
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_DRIVE);
      else if (event == REAR_CONTROLLER_EVENT_CHARGE_REQUEST)
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_CHARGE);
      else if (event == REAR_CONTROLLER_EVENT_FAULT)
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_FAULT);
      break;

    case REAR_CONTROLLER_STATE_DRIVE:
      if (event == REAR_CONTROLLER_EVENT_NEUTRAL_REQUEST) {
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_IDLE);
      } else if (event == REAR_CONTROLLER_EVENT_FAULT) {
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_FAULT);
      }
      break;

    case REAR_CONTROLLER_STATE_CHARGE:
      if (event == REAR_CONTROLLER_EVENT_CHARGER_REMOVED) {
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_IDLE);
      } else if (event == REAR_CONTROLLER_EVENT_FAULT) {
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_FAULT);
      }
      break;

    case REAR_CONTROLLER_STATE_FAULT:
      if (event == REAR_CONTROLLER_EVENT_RESET) {
        StatusCode status = relays_close_pos();
        status = relays_close_solar();
        status = relays_close_neg();

        if (status == STATUS_CODE_OK) {
          rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_IDLE);
        } else {
          rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_FAULT);
        }
      }
      break;

    default:
      rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_FAULT);
      break;
  }

  return STATUS_CODE_OK;
}

RearControllerState rear_controller_state_manager_get_state(void) {
  return s_current_state;
}
