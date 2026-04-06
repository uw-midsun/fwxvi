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
#include "front_controller_setters.h"
#include "front_controller_state_manager.h"
#include "front_lights_signal.h"
#include "power_manager.h"

/**
 * To test CAN without rear controller connected, BPS_fault and precharge_complete
 * must be disabled by setting IS_REAR_CONNECTED to 0U. Otherwise set this to 1U.
 */
#define IS_REAR_CONNECTED 1U

#define FRONT_STATE_MANAGER_DEBUG 0U

#if (FRONT_STATE_MANAGER_DEBUG == 1)
#define CONDITIONAL_LOG_DEBUG(...) LOG_DEBUG(__VA_ARGS__)
#else
#define CONDITIONAL_LOG_DEBUG(...) \
  do {                             \
  } while (0)
#endif

static FrontControllerStorage *front_controller_storage = NULL;
static FrontControllerState s_current_state = NUM_FRONT_CONTROLLER_STATES;
static bool is_horn_enabled;
static bool is_brake_enabled;
static bool started = false;

static void front_controller_state_manager_enter_state(FrontControllerState new_state) {
  switch (new_state) {
    case FRONT_CONTROLLER_STATE_IDLE:
      if (s_current_state != FRONT_CONTROLLER_STATE_IDLE || !started) {
        power_manager_set_output_group(OUTPUT_GROUP_D_R_INDICATORS, false);
        power_manager_set_output_group(OUTPUT_GROUP_IDLE, true);
      }
      break;

    case FRONT_CONTROLLER_STATE_BRAKE:
      if (s_current_state != FRONT_CONTROLLER_STATE_BRAKE || !started) {
        power_manager_set_output_group(OUTPUT_GROUP_D_R_INDICATORS, false);
        power_manager_set_output_group(OUTPUT_GROUP_IDLE, true);
      }
      break;

    case FRONT_CONTROLLER_STATE_DRIVE:
      if (s_current_state != FRONT_CONTROLLER_STATE_DRIVE || !started) {
        power_manager_set_output_group(OUTPUT_GROUP_D_R_INDICATORS, false);
        power_manager_set_output_group(OUTPUT_GROUP_DRIVE, true);
      }
      break;

    case FRONT_CONTROLLER_STATE_REVERSE:
      if (s_current_state != FRONT_CONTROLLER_STATE_DRIVE || !started) {
        power_manager_set_output_group(OUTPUT_GROUP_D_R_INDICATORS, false);
        power_manager_set_output_group(OUTPUT_GROUP_REVERSE, true);
      }
      break;

    case FRONT_CONTROLLER_STATE_FAULT:
      if (s_current_state != FRONT_CONTROLLER_STATE_FAULT || !started) {
        power_manager_set_output_group(OUTPUT_GROUP_IDLE, true);
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

  s_current_state = FRONT_CONTROLLER_STATE_IDLE;
  started = false;

  return STATUS_CODE_OK;
}

StatusCode front_controller_state_manager_step(FrontControllerEvent event) {
  if (front_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  switch (s_current_state) {
    case FRONT_CONTROLLER_STATE_IDLE:
      if (event == FRONT_CONTROLLER_EVENT_DRIVE_REQUEST) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_DRIVE);
      } else if (event == FRONT_CONTROLLER_EVENT_BRAKE_REQUEST) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_BRAKE);
      } else if (event == FRONT_CONTROLLER_EVENT_REVERSE_REQUEST) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_REVERSE);
      } else if (event == FRONT_CONTROLLER_EVENT_FAULT) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_FAULT);
      }
      break;

    case FRONT_CONTROLLER_STATE_BRAKE:
      if (event == FRONT_CONTROLLER_EVENT_DRIVE_REQUEST) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_DRIVE);
      } else if (event == FRONT_CONTROLLER_EVENT_IDLE_REQUEST) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_IDLE);
      } else if (event == FRONT_CONTROLLER_EVENT_REVERSE_REQUEST) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_REVERSE);
      } else if (event == FRONT_CONTROLLER_EVENT_FAULT) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_FAULT);
      }
      break;

    case FRONT_CONTROLLER_STATE_DRIVE:
      if (event == FRONT_CONTROLLER_EVENT_REVERSE_REQUEST) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_REVERSE);
      } else if (event == FRONT_CONTROLLER_EVENT_BRAKE_REQUEST) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_BRAKE);
      } else if (event == FRONT_CONTROLLER_EVENT_IDLE_REQUEST) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_IDLE);
      } else if (event == FRONT_CONTROLLER_EVENT_FAULT) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_FAULT);
      }
      break;

    case FRONT_CONTROLLER_STATE_REVERSE:
      if (event == FRONT_CONTROLLER_EVENT_DRIVE_REQUEST) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_DRIVE);
      } else if (event == FRONT_CONTROLLER_EVENT_BRAKE_REQUEST) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_BRAKE);
      } else if (event == FRONT_CONTROLLER_EVENT_IDLE_REQUEST) {
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
  if (started == false) {
    front_controller_state_manager_enter_state(s_current_state);
    started = true;
    return STATUS_CODE_OK;
  }

  front_controller_storage->current_drive_state = s_current_state;
  set_drive_status_state_data_drive_state(front_controller_storage->current_drive_state);

  /* Get required values from rear */
#if (IS_REAR_CONNECTED == 0U)
  uint8_t bps_fault_from_rear = 0U;
  uint8_t is_precharge_complete_from_rear = 1U;
#else
  // uint8_t bps_fault_from_rear = 0U;
  uint16_t bps_fault_from_rear = get_rear_controller_status_triggers_bps_fault();
  uint8_t is_precharge_complete_from_rear = get_rear_controller_status_triggers_motor_precharge_complete();
#endif

  /* Get required values from steering */
  uint8_t drive_state_from_steering = get_steering_buttons_drive_state();
  uint8_t lights_from_steering = get_steering_buttons_lights();
  uint8_t horn_enabled_from_steering = get_steering_buttons_horn_enabled();

  uint8_t is_regen_enabled = get_steering_buttons_regen_enabled();
  uint8_t is_cruise_control_enabled = get_steering_buttons_cruise_control_enabled();
  uint8_t is_hazard_enabled = get_steering_buttons_hazard_enabled();

  CONDITIONAL_LOG_DEBUG("STATE MANAGER MEDIUM CYCLE \r\nDS: %u LIGHTS %u HORN %u\r\n", drive_state_from_steering, lights_from_steering, horn_enabled_from_steering);

  // Handle BPS fault
  if (bps_fault_from_rear) {
    front_lights_signal_set_bps_light(BPS_LIGHT_ON_STATE);
    front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_FAULT);
    CONDITIONAL_LOG_DEBUG("Rear fault detected, front controller entering fault state\r\n");
    return STATUS_CODE_OK;
  } else if (!bps_fault_from_rear && s_current_state == FRONT_CONTROLLER_STATE_FAULT) {
    // Logic to turn off BPS fault
    front_lights_signal_set_bps_light(BPS_LIGHT_OFF_STATE);
    front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_RESET);
  } else {
  }

  // Handle brake
  if (is_brake_enabled == true && (front_controller_storage->brake_enabled == false) && (front_controller_storage->regen_enabled == false)) {
    power_manager_set_output_group(OUTPUT_GROUP_BRAKE_LIGHTS, false);
    is_brake_enabled = false;
  } else if (is_brake_enabled == false && ((front_controller_storage->brake_enabled == true) || (front_controller_storage->regen_enabled == true))) {
    power_manager_set_output_group(OUTPUT_GROUP_BRAKE_LIGHTS, true);
    front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_BRAKE_REQUEST);
    is_brake_enabled = true;
  }

  if (!is_brake_enabled && !front_controller_storage->brake_enabled) {
    // Handle drive state from steering
    if (drive_state_from_steering == VEHICLE_DRIVE_STATE_DRIVE || drive_state_from_steering == VEHICLE_DRIVE_STATE_CRUISE) {
      // Precharge MUST be complete in order to enter driving state
      if (is_precharge_complete_from_rear) {
        front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_DRIVE_REQUEST);
      } else {
        CONDITIONAL_LOG_DEBUG("Warning: incomplete precharge preventing drive\r\n");
      }
    }

    if (drive_state_from_steering == VEHICLE_DRIVE_STATE_REVERSE) {
      // Precharge MUST be complete in order to enter driving state
      if (is_precharge_complete_from_rear) {
        front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_REVERSE_REQUEST);
      } else {
        CONDITIONAL_LOG_DEBUG("Warning: incomplete precharge preventing drive\r\n");
      }
    }

    if (drive_state_from_steering == VEHICLE_DRIVE_STATE_NEUTRAL) {
      front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_IDLE_REQUEST);
    }
  }

  // Handle horn
  if (horn_enabled_from_steering && is_horn_enabled == false) {
    power_manager_set_output_group(OUTPUT_GROUP_HORN, true);
    is_horn_enabled = true;
  } else if (horn_enabled_from_steering == 0 && is_horn_enabled == true) {
    power_manager_set_output_group(OUTPUT_GROUP_HORN, false);
    is_horn_enabled = false;
  }

  // Handle lights
  if (lights_from_steering < STEERING_LIGHTS_NUM_STATES) {
    front_lights_signal_process_event(lights_from_steering);
  } else {
    CONDITIONAL_LOG_DEBUG("Warning: invalid lights state recieved from steering\r\n");
  }

  return STATUS_CODE_OK;
}
