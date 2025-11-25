#include "motor_can.h"

#include "front_controller_getters.h"

static FrontControllerStorage *frontControllerStorage;

static DriveState prv_resolve_current_state() {
  if (frontControllerStorage->brake_enabled) {
    if (get_steering_buttons_regen_braking_enabled() == 1) {
      return DRIVE_STATE_REGEN;
    } else {
      return DRIVE_STATE_BRAKE;
    }
  }

  uint8_t drive_state_from_steering = get_steering_buttons_drive_state();
  uint8_t cc_enabled_from_steering = get_steering_buttons_cruise_control();

  if (drive_state_from_steering == DRIVE_STATE_NEUTRAL) {
    return DRIVE_STATE_NEUTRAL;
  } else if (drive_state_from_steering == DRIVE_STATE_REVERSE) {
    return DRIVE_STATE_REVERSE;
  } else if (drive_state_from_steering == DRIVE_STATE_DRIVE) {
    if (cc_enabled_from_steering == 0) {
      return DRIVE_STATE_DRIVE;
    } else {
      return DRIVE_STATE_CRUISE;
    }
  }

  return DRIVE_STATE_INVALID;
}

StatusCode motor_can_update_target_current_velocity() {
  if (frontControllerStorage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  DriveState currentState = prv_resolve_current_state();

  if (currentState == DRIVE_STATE_INVALID) {
    return STATUS_CODE_INVALID_ARGS;
  }

  frontControllerStorage->currentDriveState = currentState;

  switch (currentState) {
    case DRIVE_STATE_DRIVE:
      ws22_motor_can_set_current(frontControllerStorage->accel_percentage);
      ws22_motor_can_set_velocity(WS22_CONTROLLER_MAX_VELOCITY);
      break;
    case DRIVE_STATE_REVERSE:
      ws22_motor_can_set_current(frontControllerStorage->accel_percentage);
      ws22_motor_can_set_velocity(-WS22_CONTROLLER_MAX_VELOCITY);
      break;
    case DRIVE_STATE_CRUISE:
      ws22_motor_can_set_current(1.0f);
      ws22_motor_can_set_velocity(get_target_velocity_cruise_control_target_velocity() * VEL_TO_RPM_RATIO);
      break;
    case DRIVE_STATE_BRAKE:
      ws22_motor_can_set_current(0.0f);
      ws22_motor_can_set_velocity(0.0f);
      break;
    case DRIVE_STATE_REGEN:
      ws22_motor_can_set_current(frontControllerStorage->accel_percentage);
      ws22_motor_can_set_velocity(0.0f);
      break;
    case DRIVE_STATE_NEUTRAL:
      ws22_motor_can_set_current(0.0f);
      ws22_motor_can_set_velocity(0.0f);
      break;
    default:
      // invalid drive state
      return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}

StatusCode motor_can_init(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  frontControllerStorage = storage;

  return STATUS_CODE_OK;
}
