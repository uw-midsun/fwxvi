/************************************************************************************************
 * @file    motor_can.c
 *
 * @brief   Motor Can
 *
 * @date    2025-11-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "motor_can.h"
#include "front_controller_getters.h"

static FrontControllerStorage *front_controller_storage = NULL;

static VehicleDriveState s_resolve_current_state() {
  if (front_controller_storage->brake_enabled) {
    if (get_steering_buttons_regen_braking()) {
      return VEHICLE_DRIVE_STATE_REGEN;
    } else {
      return VEHICLE_DRIVE_STATE_BRAKE;
    }
  }

  uint8_t drive_state_from_steering = get_steering_buttons_drive_state();
  uint8_t cc_enabled_from_steering = get_steering_buttons_cruise_control();

  if (drive_state_from_steering == VEHICLE_DRIVE_STATE_NEUTRAL) {
    return VEHICLE_DRIVE_STATE_NEUTRAL;
  } else if (drive_state_from_steering == VEHICLE_DRIVE_STATE_REVERSE) {
    return VEHICLE_DRIVE_STATE_REVERSE;
  } else if (drive_state_from_steering == VEHICLE_DRIVE_STATE_DRIVE) {
    if (cc_enabled_from_steering == 0) {
      return VEHICLE_DRIVE_STATE_DRIVE;
    } else {
      return VEHICLE_DRIVE_STATE_CRUISE;
    }
  }

  return VEHICLE_DRIVE_STATE_INVALID;
}

StatusCode motor_can_update_target_current_velocity() {
  if (front_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  VehicleDriveState current_state = s_resolve_current_state();

  if (current_state == VEHICLE_DRIVE_STATE_INVALID) {
    return STATUS_CODE_INVALID_ARGS;
  }

  front_controller_storage->currentDriveState = current_state;

  switch (current_state) {
    case VEHICLE_DRIVE_STATE_DRIVE:
      ws22_motor_can_set_current(front_controller_storage->accel_percentage);
      ws22_motor_can_set_velocity(WS22_CONTROLLER_MAX_VELOCITY);
      break;
    case VEHICLE_DRIVE_STATE_REVERSE:
      ws22_motor_can_set_current(front_controller_storage->accel_percentage);
      ws22_motor_can_set_velocity(-WS22_CONTROLLER_MAX_VELOCITY);
      break;
    case VEHICLE_DRIVE_STATE_CRUISE:
      ws22_motor_can_set_current(1.0f);
      ws22_motor_can_set_velocity(get_steering_target_velocity_cruise_control_target_velocity() * VEL_TO_RPM_RATIO);
      break;
    case VEHICLE_DRIVE_STATE_BRAKE:
      ws22_motor_can_set_current(0.0f);
      ws22_motor_can_set_velocity(0.0f);
      break;
    case VEHICLE_DRIVE_STATE_REGEN:
      ws22_motor_can_set_current(front_controller_storage->accel_percentage);
      ws22_motor_can_set_velocity(0.0f);
      break;
    case VEHICLE_DRIVE_STATE_NEUTRAL:
      ws22_motor_can_set_current(0.0f);
      ws22_motor_can_set_velocity(0.0f);
      break;
    default:
      /* Invalid drive state */
      return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode motor_can_init(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  front_controller_storage = storage;

  return STATUS_CODE_OK;
}
