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

#include "accel_pedal.h"
#include "front_controller_getters.h"

#define MOTOR_CAN_DEBUG 2U
#define IS_BRAKE_CONNECTED 1U

#if (MOTOR_CAN_DEBUG == 1)
static const char *print_state_str(VehicleDriveState state) {
  switch (state) {
    case VEHICLE_DRIVE_STATE_NEUTRAL:
      return "VEHICLE_DRIVE_STATE_NEUTRAL";
    case VEHICLE_DRIVE_STATE_DRIVE:
      return "VEHICLE_DRIVE_STATE_DRIVE";
    case VEHICLE_DRIVE_STATE_REVERSE:
      return "VEHICLE_DRIVE_STATE_REVERSE";
    case VEHICLE_DRIVE_STATE_CRUISE:
      return "VEHICLE_DRIVE_STATE_CRUISE";
    case VEHICLE_DRIVE_STATE_BRAKE:
      return "VEHICLE_DRIVE_STATE_BRAKE";
    case VEHICLE_DRIVE_STATE_REGEN:
      return "VEHICLE_DRIVE_STATE_REGEN";
    case VEHICLE_DRIVE_STATE_INVALID:
      return "VEHICLE_DRIVE_STATE_INVALID";
    default:
      return "UNKNOWN";
  }
}
#endif

static FrontControllerStorage *front_controller_storage = NULL;

static VehicleDriveState current_drive_state;

static VehicleDriveState s_resolve_current_state() {
#if (IS_BRAKE_CONNECTED != 0U)
  if (front_controller_storage->brake_enabled) {
    if (get_steering_buttons_regen_braking()) {
      return VEHICLE_DRIVE_STATE_REGEN;
    } else {
      return VEHICLE_DRIVE_STATE_BRAKE;
    }
  }
#endif

  VehicleDriveState drive_state_from_steering = get_steering_buttons_drive_state();
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

  current_drive_state = s_resolve_current_state();

#if (MOTOR_CAN_DEBUG == 1)
  LOG_DEBUG("resolve_current_state returned %s\n", print_state_str(current_drive_state));
#endif

  if (current_drive_state == VEHICLE_DRIVE_STATE_INVALID) {
    return STATUS_CODE_INVALID_ARGS;
  }

  front_controller_storage->currentDriveState = current_drive_state;

  switch (current_drive_state) {
    case VEHICLE_DRIVE_STATE_DRIVE:
#if (MOTOR_CAN_DEBUG == 2)
      LOG_DEBUG("DRIVE, Accel percentage: %ld\r\n", (int32_t)(front_controller_storage->accel_pedal_storage->accel_percentage * 100));
#endif
      ws22_motor_can_set_current(front_controller_storage->accel_pedal_storage->accel_percentage);
      ws22_motor_can_set_velocity(WS22_CONTROLLER_MAX_VELOCITY);
      break;
    case VEHICLE_DRIVE_STATE_REVERSE:
#if (MOTOR_CAN_DEBUG == 2)
      LOG_DEBUG("REVERSE Accel percentage: %ld\r\n", (int32_t)(front_controller_storage->accel_pedal_storage->accel_percentage * 100));
#endif
      ws22_motor_can_set_current(front_controller_storage->accel_pedal_storage->accel_percentage);
      ws22_motor_can_set_velocity((-1) * WS22_CONTROLLER_MAX_VELOCITY);
      break;
    case VEHICLE_DRIVE_STATE_CRUISE:
#if (MOTOR_CAN_DEBUG == 2)
      LOG_DEBUG("CRUISE, CC velocity: %ld\r\n", (int32_t)(get_steering_target_velocity_cruise_control_target_velocity() * VEL_TO_RPM_RATIO));
#endif
      ws22_motor_can_set_current(1.0f);
      ws22_motor_can_set_velocity(get_steering_target_velocity_cruise_control_target_velocity() * VEL_TO_RPM_RATIO);
      break;
    case VEHICLE_DRIVE_STATE_BRAKE:
      LOG_DEBUG("Braking:\r\n");
      ws22_motor_can_set_current(0.0f);
      ws22_motor_can_set_velocity(0.0f);
      break;
    case VEHICLE_DRIVE_STATE_REGEN:
#if (MOTOR_CAN_DEBUG == 2)
      LOG_DEBUG("Accel percentage: %ld\r\n", (int32_t)(front_controller_storage->accel_pedal_storage->accel_percentage * 100));
#endif
      ws22_motor_can_set_current(front_controller_storage->accel_pedal_storage->accel_percentage);
      ws22_motor_can_set_velocity(0.0f);
      break;
    case VEHICLE_DRIVE_STATE_NEUTRAL:
      LOG_DEBUG("Neutral:\r\n");
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

StatusCode motor_can_get_current_state(VehicleDriveState *current_state) {
  *current_state = current_drive_state;
  return STATUS_CODE_OK;
}

char *motor_can_get_current_state_str() {
  switch (current_drive_state) {
    case VEHICLE_DRIVE_STATE_NEUTRAL:
      return "VEHICLE_DRIVE_STATE_NEUTRAL";
    case VEHICLE_DRIVE_STATE_DRIVE:
      return "VEHICLE_DRIVE_STATE_DRIVE";
    case VEHICLE_DRIVE_STATE_REVERSE:
      return "VEHICLE_DRIVE_STATE_REVERSE";
    case VEHICLE_DRIVE_STATE_CRUISE:
      return "VEHICLE_DRIVE_STATE_CRUISE";
    case VEHICLE_DRIVE_STATE_BRAKE:
      return "VEHICLE_DRIVE_STATE_BRAKE";
    case VEHICLE_DRIVE_STATE_REGEN:
      return "VEHICLE_DRIVE_STATE_REGEN";
    case VEHICLE_DRIVE_STATE_INVALID:
      return "VEHICLE_DRIVE_STATE_INVALID";
    default:
      return "UNKNOWN";
  }
}
