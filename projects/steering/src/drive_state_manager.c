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
#include "drive_state_manager.h"

/**
 * To test drive_state_manager without rear controller connected, BPS_fault and precharge_complete
 * must be disabled by setting IS_REAR_CONNECTED to 0U. Otherwise set this to 1U.
 */
#define IS_REAR_CONNECTED 0U

static DriveState current_state = DRIVE_STATE_NEUTRAL;
static DriveStateRequest current_request = DRIVE_STATE_REQUEST_NONE;

static StatusCode drive_state_manager_neutral(void) {
  if (current_state == DRIVE_STATE_DRIVE) {
    button_manager_led_disable(STEERING_BUTTON_DRIVE);
  } else if (current_state == DRIVE_STATE_REVERSE) {
    button_manager_led_disable(STEERING_BUTTON_REVERSE);
  }

  button_manager_led_enable(STEERING_BUTTON_NEUTRAL);
  set_steering_buttons_drive_state(VEHICLE_DRIVE_STATE_NEUTRAL);
  return STATUS_CODE_OK;
}

static StatusCode drive_state_manager_reverse(void) {
  if (current_state == DRIVE_STATE_DRIVE && current_request == DRIVE_STATE_REQUEST_R) {
    LOG_DEBUG("Cannot change state to REVERSE from DRIVE\n");

    current_request = DRIVE_STATE_REQUEST_NONE;
    return STATUS_CODE_INVALID_ARGS;
  }

  button_manager_led_disable(STEERING_BUTTON_NEUTRAL);
  button_manager_led_enable(STEERING_BUTTON_REVERSE);

#if (IS_REAR_CONNECTED != 0U)
  // If the bps has faulted
  if (get_rear_controller_status_bps_fault() != 0) {
    LOG_DEBUG("Cannot change state; BPS has faulted\n");

    return STATUS_CODE_RESOURCE_EXHAUSTED;

    // If the vehicle is not ready
  } else if (get_rear_controller_status_power_state() != VEHICLE_POWER_STATE_IDLE) {
    LOG_DEBUG("Cannot change state; Vehicle is not in idle state\n");

    return STATUS_CODE_RESOURCE_EXHAUSTED;

    // If the precharge is incomplete
  } else if (get_battery_stats_B_motor_precharge_complete() == 0) {
    LOG_DEBUG("Cannot change state; Precharge is not complete\n");

    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
#endif

  set_steering_buttons_drive_state(VEHICLE_DRIVE_STATE_REVERSE);
  LOG_DEBUG("Setting drive state to REVERSE\n");
  return STATUS_CODE_OK;
}

static StatusCode drive_state_manager_drive(void) {
  if (current_state == DRIVE_STATE_REVERSE && current_request == DRIVE_STATE_REQUEST_D) {
    LOG_DEBUG("Cannot change state to DRIVE from REVERSE\n");

    current_request = DRIVE_STATE_REQUEST_NONE;
    return STATUS_CODE_INVALID_ARGS;
  }

  button_manager_led_disable(STEERING_BUTTON_NEUTRAL);
  button_manager_led_enable(STEERING_BUTTON_DRIVE);

#if (IS_REAR_CONNECTED != 0U)
  // If the bps has faulted
  if (get_rear_controller_status_bps_fault() != 0) {
    LOG_DEBUG("Cannot change state; BPS has faulted\n");

    return STATUS_CODE_RESOURCE_EXHAUSTED;

    // If the vehicle is not ready
  } else if (get_rear_controller_status_power_state() != VEHICLE_POWER_STATE_IDLE) {
    LOG_DEBUG("Cannot change state; Vehicle is not in idle state\n");

    return STATUS_CODE_RESOURCE_EXHAUSTED;

    // If the precharge is incomplete
  } else if (get_battery_stats_B_motor_precharge_complete() == 0) {
    LOG_DEBUG("Cannot change state; Precharge is not complete\n");

    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
#endif

  set_steering_buttons_drive_state(VEHICLE_DRIVE_STATE_DRIVE);
  LOG_DEBUG("Setting drive state to DRIVE\n");
  return STATUS_CODE_OK;
}

StatusCode drive_state_manager_init(void) {
  current_state = DRIVE_STATE_NEUTRAL;
  current_request = DRIVE_STATE_REQUEST_NONE;
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

      if (current_state != DRIVE_STATE_DRIVE && drive_state_manager_drive() == STATUS_CODE_OK) {
        LOG_DEBUG("Drive state set to DRIVE\n");
        current_state = DRIVE_STATE_DRIVE;
      }

      break;

    case DRIVE_STATE_REQUEST_N:

    if (drive_state_manager_neutral() == STATUS_CODE_OK && current_state != DRIVE_STATE_NEUTRAL) {
        LOG_DEBUG("Drive state set to NEUTRAL\n");
        current_state = DRIVE_STATE_NEUTRAL;
      }
      break;

    case DRIVE_STATE_REQUEST_R:

      if (current_state != DRIVE_STATE_REVERSE && drive_state_manager_reverse() == STATUS_CODE_OK) {
        LOG_DEBUG("Drive state set to REVERSE\n");
        current_state = DRIVE_STATE_REVERSE;
      }

      break;

    case DRIVE_STATE_REQUEST_NONE:
      break;

    default:
      current_state = DRIVE_STATE_INVALID;
      return STATUS_CODE_INVALID_ARGS;
      break;
  }

  return STATUS_CODE_OK;
}

DriveState drive_state_manager_get_state(void) {
  return current_state;
}
