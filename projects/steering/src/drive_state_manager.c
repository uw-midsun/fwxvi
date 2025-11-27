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

static DriveState current_state = DRIVE_STATE_NEUTRAL;
static DriveStateRequest current_request = DRIVE_STATE_REQUEST_NONE;

static StatusCode drive_state_manager_neutral(void) {
  if (current_state == DRIVE_STATE_DRIVE) {
    button_manager_led_disable(STEERING_BUTTON_NEUTRAL);
  } else if (current_state == DRIVE_STATE_REVERSE) {
    button_manager_led_disable(STEERING_BUTTON_REVERSE);
  }

  button_manager_led_enable(STEERING_BUTTON_NEUTRAL);
  set_steering_buttons_drive_state(VEHICLE_NEUTRAL);
  return STATUS_CODE_OK;
}

static StatusCode drive_state_manager_reverse(void) {
  if (current_state == DRIVE_STATE_DRIVE) {
    LOG_DEBUG("Cannot change state to REVERSE from DRIVE\n");
    return STATUS_CODE_INVALID_ARGS;
  }

  button_manager_led_disable(STEERING_BUTTON_NEUTRAL);
  button_manager_led_enable(STEERING_BUTTON_REVERSE);

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

  set_steering_buttons_drive_state(VEHICLE_REVERSE);
  LOG_DEBUG("Setting drive state to REVERSE\n");
  return STATUS_CODE_OK;
}

static StatusCode drive_state_manager_drive(void) {
  if (current_state == DRIVE_STATE_REVERSE) {
    LOG_DEBUG("Cannot change state to DRIVE from REVERSE\n");
    return STATUS_CODE_INVALID_ARGS;
  }

  button_manager_led_disable(STEERING_BUTTON_NEUTRAL);
  button_manager_led_enable(STEERING_BUTTON_DRIVE);

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

  set_steering_buttons_drive_state(VEHICLE_DRIVE);
  LOG_DEBUG("Setting drive state to DRIVE\n");
  return STATUS_CODE_OK;
}

void drive_state_manager_init(void) {
  current_state = DRIVE_STATE_NEUTRAL;
  current_request = DRIVE_STATE_REQUEST_NONE;
}

void drive_state_manager_request(DriveStateRequest req) {
  current_request = req;
}

void drive_state_manager_update(void) {
  switch (current_request) {
    case DRIVE_STATE_REQUEST_D:

      if (drive_state_manager_drive() == STATUS_CODE_OK) {
        LOG_DEBUG("Drive state set to DRIVE\n");
        current_state = DRIVE_STATE_DRIVE;
      }

      break;

    case DRIVE_STATE_REQUEST_N:

      drive_state_manager_neutral();

      LOG_DEBUG("Drive state set to NEUTRAL\n");
      current_state = DRIVE_STATE_NEUTRAL;
      break;

    case DRIVE_STATE_REQUEST_R:

      if (drive_state_manager_reverse() == STATUS_CODE_OK) {
        LOG_DEBUG("Drive state set to REVERSE\n");
        current_state = DRIVE_STATE_REVERSE;
      }

      break;

    default:
      current_state = DRIVE_STATE_INVALID;
      break;
  }

  LOG_DEBUG("Drive State Manager is in the state: %d\n", current_state);
}

DriveState drive_state_manager_get_state(void) {
  return current_state;
}
