/************************************************************************************************
 * @file   drive_state_manager.c
 *
 * @brief  Source file for drive state manager
 *
 * @date   2025-07-22
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "button_manager.h"
#include "global_enums.h"
#include "steering_getters.h"
#include "steering_setters.h"

/* Intra-component Headers */
#include "button_led.h"
#include "buzzer.h"
#include "drive_state_manager.h"
#include "steering.h"

/**
 * To test drive_state_manager without rear controller connected, BPS_fault and precharge_complete
 * must be disabled by setting IS_REAR_CONNECTED to 0U. Otherwise set this to 1U.
 */
#define IS_REAR_CONNECTED 0U
#define DRIVE_STATE_MANAGER_DEBUG 0U

static VehicleDriveState current_state = VEHICLE_DRIVE_STATE_INVALID;
static DriveStateRequest current_request = DRIVE_STATE_REQUEST_NONE;
static RegenState current_regen_state = INVALID_REGEN_STATE;

static SteeringStorage *steering_storage = NULL;

static StatusCode drive_state_manager_neutral(void) {
  if (current_state == VEHICLE_DRIVE_STATE_DRIVE) {
    button_led_disable(STEERING_BUTTON_DRIVE);
  } else if (current_state == VEHICLE_DRIVE_STATE_REVERSE) {
    button_led_disable(STEERING_BUTTON_REVERSE);
  }

  button_led_enable(STEERING_BUTTON_NEUTRAL);
  buzzer_play_neutral();
  set_steering_buttons_drive_state(VEHICLE_DRIVE_STATE_NEUTRAL);
  return STATUS_CODE_OK;
}

static StatusCode drive_state_manager_reverse(void) {
  if (current_state == VEHICLE_DRIVE_STATE_DRIVE && current_request == DRIVE_STATE_REQUEST_R) {
#if (DRIVE_STATE_MANAGER_DEBUG == 1)
    LOG_DEBUG("Cannot change state to REVERSE from DRIVE\n");
#endif
    buzzer_play_invalid();

    current_request = DRIVE_STATE_REQUEST_NONE;
    return STATUS_CODE_INVALID_ARGS;
  }

#if (IS_REAR_CONNECTED != 0U)
  // If the bps has faulted
  if (get_rear_controller_status_bps_fault() != 0) {
#if (DRIVE_STATE_MANAGER_DEBUG == 1)
    LOG_DEBUG("Cannot change state; BPS has faulted\n");
#endif
    buzzer_play_invalid();

    return STATUS_CODE_RESOURCE_EXHAUSTED;

    // If the vehicle is not ready
  } else if (get_rear_controller_status_power_state() != VEHICLE_POWER_STATE_IDLE) {
#if (DRIVE_STATE_MANAGER_DEBUG == 1)
    LOG_DEBUG("Cannot change state; Vehicle is not in idle state\n");
#endif
    buzzer_play_invalid();

    return STATUS_CODE_RESOURCE_EXHAUSTED;

    // If the precharge is incomplete
  } else if (get_battery_stats_B_motor_precharge_complete() == 0) {
#if (DRIVE_STATE_MANAGER_DEBUG == 1)
    LOG_DEBUG("Cannot change state; Precharge is not complete\n");
#endif
    buzzer_play_invalid();

    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
#endif
  button_led_disable(STEERING_BUTTON_NEUTRAL);
  button_led_enable(STEERING_BUTTON_REVERSE);
  buzzer_play_reverse();

  set_steering_buttons_drive_state(VEHICLE_DRIVE_STATE_REVERSE);
#if (DRIVE_STATE_MANAGER_DEBUG == 1)
  LOG_DEBUG("Setting drive state to REVERSE\n");
#endif
  return STATUS_CODE_OK;
}

static StatusCode drive_state_manager_drive(void) {
  if (current_state == VEHICLE_DRIVE_STATE_REVERSE && current_request == DRIVE_STATE_REQUEST_D) {
#if (DRIVE_STATE_MANAGER_DEBUG == 1)
    LOG_DEBUG("Cannot change state to DRIVE from REVERSE\n");
#endif
    buzzer_play_invalid();

    current_request = DRIVE_STATE_REQUEST_NONE;
    return STATUS_CODE_INVALID_ARGS;
  }

#if (IS_REAR_CONNECTED != 0U)
  // If the bps has faulted
  if (get_rear_controller_status_bps_fault() != 0) {
#if (DRIVE_STATE_MANAGER_DEBUG == 1)
    LOG_DEBUG("Cannot change state; BPS has faulted\n");
#endif
    buzzer_play_invalid();

    return STATUS_CODE_RESOURCE_EXHAUSTED;

    // If the vehicle is not ready
  } else if (get_rear_controller_status_power_state() != VEHICLE_POWER_STATE_IDLE) {
#if (DRIVE_STATE_MANAGER_DEBUG == 1)
    LOG_DEBUG("Cannot change state; Vehicle is not in idle state\n");
#endif
    buzzer_play_invalid();

    return STATUS_CODE_RESOURCE_EXHAUSTED;

    // If the precharge is incomplete
  } else if (get_battery_stats_B_motor_precharge_complete() == 0) {
#if (DRIVE_STATE_MANAGER_DEBUG == 1)
    LOG_DEBUG("Cannot change state; Precharge is not complete\n");
#endif
    buzzer_play_invalid();

    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
#endif

  button_led_disable(STEERING_BUTTON_NEUTRAL);
  button_led_enable(STEERING_BUTTON_DRIVE);
  buzzer_play_drive();

  set_steering_buttons_drive_state(VEHICLE_DRIVE_STATE_DRIVE);
#if (DRIVE_STATE_MANAGER_DEBUG == 1)
  LOG_DEBUG("Setting drive state to DRIVE\n");
#endif
  return STATUS_CODE_OK;
}

StatusCode drive_state_manager_init(SteeringStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  steering_storage = storage;

  current_state = VEHICLE_DRIVE_STATE_INVALID;
  current_request = DRIVE_STATE_REQUEST_NONE;
  current_regen_state = REGEN_STATE_DISABLED;

  drive_state_manager_request(DRIVE_STATE_REQUEST_N);
  drive_state_manager_enter_regen_state(REGEN_STATE_DISABLED);
  return STATUS_CODE_OK;
}

StatusCode drive_state_manager_request(DriveStateRequest req) {
  if (req >= NUM_DRIVE_STATE_REQUESTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  current_request = req;
  return STATUS_CODE_OK;
}

StatusCode drive_state_manager_update(void) {
  switch (current_request) {
    case DRIVE_STATE_REQUEST_D:

      if (current_state != VEHICLE_DRIVE_STATE_DRIVE) {
        StatusCode ret = drive_state_manager_drive();
        if (ret == STATUS_CODE_OK) {
#if (DRIVE_STATE_MANAGER_DEBUG == 1)
          LOG_DEBUG("Drive state set to DRIVE\n");
#endif
          current_state = VEHICLE_DRIVE_STATE_DRIVE;
          current_request = DRIVE_STATE_REQUEST_NONE;
        }
      }

      break;

    case DRIVE_STATE_REQUEST_N:

      if (current_state != VEHICLE_DRIVE_STATE_NEUTRAL) {
        StatusCode ret = drive_state_manager_neutral();
        if (ret == STATUS_CODE_OK) {
#if (DRIVE_STATE_MANAGER_DEBUG == 1)
          LOG_DEBUG("Drive state set to NEUTRAL\n");
#endif
          current_state = VEHICLE_DRIVE_STATE_NEUTRAL;
          steering_storage->cruise_control_enabled = false;
          set_steering_buttons_cruise_control(steering_storage->cruise_control_enabled);
          current_request = DRIVE_STATE_REQUEST_NONE;
        }
      }
      break;

    case DRIVE_STATE_REQUEST_R:

      if (current_state != VEHICLE_DRIVE_STATE_REVERSE) {
        StatusCode ret = drive_state_manager_reverse();
        if (ret == STATUS_CODE_OK) {
#if (DRIVE_STATE_MANAGER_DEBUG == 1)
          LOG_DEBUG("Drive state set to REVERSE\n");
#endif
          current_state = VEHICLE_DRIVE_STATE_REVERSE;
          steering_storage->cruise_control_enabled = false;
          set_steering_buttons_cruise_control(steering_storage->cruise_control_enabled);
          current_request = DRIVE_STATE_REQUEST_NONE;
        }
      }

      break;

    case DRIVE_STATE_REQUEST_NONE:
      break;

    default:
      current_state = VEHICLE_DRIVE_STATE_INVALID;
      return STATUS_CODE_INVALID_ARGS;
      break;
  }

  return STATUS_CODE_OK;
}

VehicleDriveState drive_state_manager_get_state(void) {
  return current_state;
}

StatusCode drive_state_manager_enter_regen_state(RegenState new_regen_state) {
  if (new_regen_state >= INVALID_REGEN_STATE) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if ((current_regen_state != REGEN_STATE_ENABLED) && (new_regen_state == REGEN_STATE_ENABLED)) {
    set_steering_buttons_regen_braking(REGEN_STATE_ENABLED);
    button_led_enable(STEERING_BUTTON_REGEN);
    buzzer_play_regen_on();
  } else if ((current_regen_state != REGEN_STATE_DISABLED) && (new_regen_state == REGEN_STATE_DISABLED)) {
    set_steering_buttons_regen_braking(REGEN_STATE_DISABLED);
    button_led_disable(STEERING_BUTTON_REGEN);
    buzzer_play_regen_off();
  }

  current_regen_state = new_regen_state;

  return STATUS_CODE_OK;
}

StatusCode drive_state_manager_toggle_regen() {
  if (current_regen_state == INVALID_REGEN_STATE) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (current_regen_state == REGEN_STATE_DISABLED) {
    status_ok_or_return(drive_state_manager_enter_regen_state(REGEN_STATE_ENABLED));
  } else if (current_regen_state == REGEN_STATE_ENABLED) {
    status_ok_or_return(drive_state_manager_enter_regen_state(REGEN_STATE_DISABLED));
  }

  return STATUS_CODE_OK;
}
