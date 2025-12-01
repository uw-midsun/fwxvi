/***********************************************************************************************
 * @file    front_controller_state_manager.c
 *
 * @brief   Front Controller state machine manager source file
 *
 * @date    2025-09-14
 * @author  Midnight Sun Team #24 - MSXVI
 ***********************************************************************************************/

/* Standard library Headers */
#include <stdio.h>

/* Inter-component Headers */
#include "log.h"

/* Intra-component Headers */
#include "front_controller_getters.h"
#include "front_controller_state_manager.h"
#include "front_lights_signal.h"
#include "power_manager.h"

static FrontControllerStorage *front_controller_storage = NULL;
static FrontControllerState s_current_state = NUM_FRONT_CONTROLLER_STATES;
static bool isHornEnabled = 0;

static void front_controller_state_manager_enter_state(FrontControllerState new_state) {
  switch (new_state) {
    case FRONT_CONTROLLER_STATE_IDLE:
      if (s_current_state != FRONT_CONTROLLER_STATE_IDLE) {
        power_manager_set_output_group(OUTPUT_GROUP_ALL, false);
        power_manager_set_output_group(OUTPUT_GROUP_IDLE, true);
      }
      break;

    case FRONT_CONTROLLER_STATE_ENGAGED:
      if (s_current_state != FRONT_CONTROLLER_STATE_ENGAGED) {
        power_manager_set_output_group(OUTPUT_GROUP_ALL, true);
      }
      break;

    case FRONT_CONTROLLER_STATE_FAULT:
      if (s_current_state != FRONT_CONTROLLER_STATE_FAULT) {
        power_manager_set_output_group(OUTPUT_GROUP_ALL, false);
        power_manager_set_output_group(OUTPUT_GROUP_HAZARD_LIGHTS, false);
      }
      break;

    default:
      break;
  }

  s_current_state = new_state;
}

StatusCode front_controller_state_manager_init(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  front_controller_storage = storage;

  front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_IDLE);
  isHornEnabled = 0;
  return STATUS_CODE_OK;
}

StatusCode front_controller_state_manager_step(FrontControllerEvent event) {
  if (front_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  switch (s_current_state) {
    case FRONT_CONTROLLER_STATE_IDLE:
      if (event == FRONT_CONTROLLER_EVENT_DRIVE_REQUEST) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_ENGAGED);
      } else if (event == FRONT_CONTROLLER_EVENT_FAULT) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_FAULT);
      }
      break;

    case FRONT_CONTROLLER_STATE_ENGAGED:
      if (event == FRONT_CONTROLLER_EVENT_IDLE_REQUEST) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_IDLE);
      } else if (event == FRONT_CONTROLLER_EVENT_FAULT) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_FAULT);
      }
      break;

    case FRONT_CONTROLLER_STATE_FAULT:
      if (event == FRONT_CONTROLLER_EVENT_RESET) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_IDLE);
      }
      break;

    default:
      front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_FAULT);
      break;
  }

  return STATUS_CODE_OK;
}

FrontControllerState front_controller_state_manager_get_state(void) {
  return s_current_state;
}

StatusCode front_controller_update_state_manager_medium_cycle() {
  uint8_t bps_fault_from_rear = get_rear_controller_status_bps_fault();
  uint8_t drive_state_from_steering = get_steering_buttons_drive_state();
  uint8_t is_precharge_complete_from_rear = get_battery_stats_B_motor_precharge_complete();
  uint8_t lights_from_rear = get_steering_buttons_lights();
  uint8_t horn_enabled_from_rear = get_steering_buttons_horn_enabled();

  if (bps_fault_from_rear == 1) {
    front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_FAULT);
    LOG_DEBUG("Rear fault detected, front controller entering fault state\r\n");
    return STATUS_CODE_OK;
  }

  if (drive_state_from_steering == VEHICLE_DRIVE_STATE_DRIVE || drive_state_from_steering == VEHICLE_DRIVE_STATE_CRUISE || drive_state_from_steering == VEHICLE_DRIVE_STATE_REVERSE) {
    if (is_precharge_complete_from_rear == 1) {
      front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_DRIVE_REQUEST);
    } else {
      LOG_DEBUG("Warning: incomplete precharge preventing drive\r\n");
    }
  }

  if (drive_state_from_steering == VEHICLE_DRIVE_STATE_NEUTRAL) {
    front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_IDLE_REQUEST);
  }

  if (horn_enabled_from_rear == 1) {
    if (isHornEnabled != 1) {
      power_manager_set_output_group(OUTPUT_GROUP_HORN, true);
      isHornEnabled = 1;
    }
  } else {
    if (isHornEnabled != 0) {
      power_manager_set_output_group(OUTPUT_GROUP_HORN, false);
      isHornEnabled = 0;
    }
  }

  if (lights_from_rear == STEERING_LIGHTS_OFF_STATE) {
    front_lights_signal_process_event(STEERING_LIGHTS_OFF_STATE);
  } else if (lights_from_rear == STEERING_LIGHTS_LEFT_STATE) {
    front_lights_signal_process_event(STEERING_LIGHTS_LEFT_STATE);
  } else if (lights_from_rear == STEERING_LIGHTS_RIGHT_STATE) {
    front_lights_signal_process_event(STEERING_LIGHTS_RIGHT_STATE);
  } else if (STEERING_LIGHTS_HAZARD_STATE) {
    front_lights_signal_process_event(STEERING_LIGHTS_HAZARD_STATE);
  } else {
    LOG_DEBUG("Warning: invalid lights state recieved from steering\r\n");
  }

  return STATUS_CODE_OK;
}
