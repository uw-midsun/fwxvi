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
#include "global_enums.h"
#include "rear_controller_getters.h"
#include "rear_controller_safety_limits.h"
#include "rear_controller_state_manager.h"
#include "relays.h"

static RearControllerStorage *rear_controller_storage = NULL;
static RearControllerState s_current_state = REAR_CONTROLLER_STATE_IDLE;

static StatusCode status;
static bool started = false;

#define IS_MOTOR_CONNECTED 1U
#define REAR_STATE_MANAGER_DEBUG 0U

#if (REAR_STATE_MANAGER_DEBUG == 1)
#define CONDITIONAL_LOG_DEBUG(...) LOG_DEBUG(__VA_ARGS__)
#else
#define CONDITIONAL_LOG_DEBUG(...) \
  do {                             \
  } while (0)
#endif

/**
 * @brief   Asynchronous event handler
 */
static void rear_controller_state_manager_enter_state(RearControllerState new_state) {
  s_current_state = new_state;

  switch (new_state) {
    case REAR_CONTROLLER_STATE_IDLE:
      relays_disable_ws22_lv();
      relays_open_motor();
      break;

    case REAR_CONTROLLER_STATE_DRIVE:
#if (IS_MOTOR_CONNECTED == 1)
      if (rear_controller_storage->precharge_complete) {
        relays_enable_ws22_lv();
        relays_close_motor();
      } else {
        rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_NEUTRAL_REQUEST);
      }
#else
      relays_enable_ws22_lv();
      relays_close_motor();
#endif
      break;

    case REAR_CONTROLLER_STATE_FAULT:
      relays_disable_ws22_lv();
      relays_reset();
      break;
    case REAR_CONTROLLER_STATE_START:
      relays_close_pos();
      relays_close_solar();
      relays_close_neg();
      started = true;
      break;
  }
}

StatusCode rear_controller_state_manager_init(RearControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;
  started = false;
  s_current_state = REAR_CONTROLLER_STATE_START;

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

    case REAR_CONTROLLER_STATE_FAULT:
      if (event == REAR_CONTROLLER_EVENT_RESET) {
        StatusCode status = relays_close_pos();
        status = relays_close_solar();
        status = relays_close_neg();

        if (status == STATUS_CODE_OK) {
          rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_IDLE);
        } else {
          CONDITIONAL_LOG_DEBUG("Reset was unsuccessful\n");
          rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_FAULT);
        }
      }
      break;

    case REAR_CONTROLLER_STATE_START:
      if (event == REAR_CONTROLLER_EVENT_NEUTRAL_REQUEST) {
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_IDLE);
      } else if (event == REAR_CONTROLLER_EVENT_DRIVE_REQUEST) {
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_DRIVE);
      } else if (event == REAR_CONTROLLER_EVENT_FAULT) {
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_FAULT);
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

StatusCode rear_controller_update_state_manager_medium_cycle() {
  CONDITIONAL_LOG_DEBUG("Current state: %d\r\n", s_current_state);
  if (s_current_state == REAR_CONTROLLER_STATE_START && started == false) {
    rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_START);
    return STATUS_CODE_OK;
  }

  if (rear_controller_storage->bps_fault != 0 && s_current_state != REAR_CONTROLLER_STATE_FAULT) {
    rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_FAULT);
    return STATUS_CODE_OK;
  }

  uint8_t drive_state_from_steering = get_steering_buttons_drive_state();
  uint8_t drive_state_from_front = get_drive_status_state_data_drive_state();

  if (drive_state_from_front == VEHICLE_DRIVE_STATE_BRAKE && s_current_state != REAR_CONTROLLER_STATE_IDLE) {
    rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_NEUTRAL_REQUEST);
  } else if (s_current_state != REAR_CONTROLLER_STATE_DRIVE &&
             (drive_state_from_steering == VEHICLE_DRIVE_STATE_DRIVE || drive_state_from_steering == VEHICLE_DRIVE_STATE_CRUISE || drive_state_from_steering == VEHICLE_DRIVE_STATE_REVERSE)) {
#if (IS_MOTOR_CONNECTED == 1)
    if (rear_controller_storage->precharge_complete) {
      rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_DRIVE_REQUEST);
    }
#else
    rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_DRIVE_REQUEST);
#endif
  } else if (drive_state_from_steering == VEHICLE_DRIVE_STATE_NEUTRAL && s_current_state != REAR_CONTROLLER_STATE_IDLE) {
    rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_NEUTRAL_REQUEST);
  }
  return STATUS_CODE_OK;
}
