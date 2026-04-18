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
#include "cruise_control.h"
#include "front_controller_getters.h"
#include "front_controller_setters.h"
#include "regen_brake.h"

#define MOTOR_CAN_DEBUG 0U

#if (MOTOR_CAN_DEBUG == 1)
#define CONDITIONAL_LOG_DEBUG(...) LOG_DEBUG(__VA_ARGS__)
#else
#define CONDITIONAL_LOG_DEBUG(...) \
  do {                             \
  } while (0)
#endif

#define IS_BRAKE_CONNECTED 0U

static FrontControllerStorage *front_controller_storage = NULL;

static VehicleDriveState current_drive_state;

static float regen_strength;
static bool regen_direction;

StatusCode motor_can_update_target_current_velocity() {
  if (front_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  current_drive_state = front_controller_storage->current_drive_state;

  if (current_drive_state == VEHICLE_DRIVE_STATE_INVALID) {
    return STATUS_CODE_INVALID_ARGS;
  }

  switch (current_drive_state) {
    case VEHICLE_DRIVE_STATE_DRIVE:
      CONDITIONAL_LOG_DEBUG("DRIVE, Accel percentage: %ld\r\n", (int32_t)(front_controller_storage->accel_pedal_storage->accel_percentage * 100));
      ws22_motor_can_set_current(front_controller_storage->accel_pedal_storage->accel_percentage);
      ws22_motor_can_set_velocity((-1) * WS22_CONTROLLER_MAX_VELOCITY);
      break;
    case VEHICLE_DRIVE_STATE_REVERSE:
      CONDITIONAL_LOG_DEBUG("REVERSE Accel percentage: %ld\r\n", (int32_t)(front_controller_storage->accel_pedal_storage->accel_percentage * 100));
      ws22_motor_can_set_current(front_controller_storage->accel_pedal_storage->accel_percentage);
      ws22_motor_can_set_velocity(WS22_CONTROLLER_MAX_VELOCITY);
      break;
    case VEHICLE_DRIVE_STATE_CRUISE:
      CONDITIONAL_LOG_DEBUG("CRUISE, CC RPM: %ld\r\n", (int32_t)(get_steering_cruise_control_target_velocity()));
      if (front_controller_storage->cruise_control_storage->target_motor_velocity != get_steering_cruise_control_target_velocity()) {
        front_controller_storage->cruise_control_storage->target_motor_velocity = get_steering_cruise_control_target_velocity();
        front_controller_storage->cruise_control_storage->set_current = front_controller_storage->ws22_motor_can_storage->control.current;
      }
      cruise_control_run();
      ws22_motor_can_set_current(front_controller_storage->cruise_control_storage->set_current);
      ws22_motor_can_set_velocity(WS22_CONTROLLER_MAX_VELOCITY);
      break;
    case VEHICLE_DRIVE_STATE_BRAKE:
      CONDITIONAL_LOG_DEBUG("Braking\r\n");
      ws22_motor_can_set_current(0.0f);
      ws22_motor_can_set_velocity(0.0f);
      break;
    case VEHICLE_DRIVE_STATE_REGEN:
      regen_brake_run(&regen_strength, &regen_direction);
      CONDITIONAL_LOG_DEBUG("REGEN STRENGTH: %ld\r\n", (int32_t)(regen_strength * 100));

      if (regen_strength == 0) {
        ws22_motor_can_set_current(0.0f);
        ws22_motor_can_set_velocity(0.0f);
      } else if (regen_direction == true) {
        ws22_motor_can_set_current(regen_strength);
        ws22_motor_can_set_velocity(WS22_CONTROLLER_MAX_VELOCITY);
      } else {
        ws22_motor_can_set_current(regen_strength);
        ws22_motor_can_set_velocity((-1) * WS22_CONTROLLER_MAX_VELOCITY);
      }

      break;
    case VEHICLE_DRIVE_STATE_NEUTRAL:
      CONDITIONAL_LOG_DEBUG("Neutral\r\n");
      ws22_motor_can_set_current(0.0f);
      ws22_motor_can_set_velocity(0.0f);
      break;
    default:
      /* Invalid drive state */
      return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode motor_can_forward_can_data() {
  set_motor_stats_B_vehicle_velocity((int16_t)front_controller_storage->vehicle_speed_kph);
  set_motor_stats_B_motor_velocity((int16_t)front_controller_storage->ws22_motor_can_storage->telemetry.motor_velocity);
  
  set_motor_stats_A_bus_voltage((int16_t)front_controller_storage->ws22_motor_can_storage->telemetry.bus_voltage);
  set_motor_stats_A_bus_current((int16_t)front_controller_storage->ws22_motor_can_storage->telemetry.bus_current);
  set_motor_stats_A_rail_15v_supply((int16_t)front_controller_storage->ws22_motor_can_storage->telemetry.rail_15v_supply);
  set_motor_stats_A_flags(front_controller_storage->ws22_motor_can_storage->telemetry.merged_flags);
  
  set_motor_stats_B_heat_sink_temp((int16_t)front_controller_storage->ws22_motor_can_storage->telemetry.heat_sink_temp);
  set_motor_stats_B_motor_temp((int16_t)front_controller_storage->ws22_motor_can_storage->telemetry.motor_temp);
  
  LOG_DEBUG("MOT V: %d | MotVel: %d\r\n", (int8_t)front_controller_storage->ws22_motor_can_storage->telemetry.bus_voltage, (int16_t)front_controller_storage->ws22_motor_can_storage->telemetry.motor_velocity);

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
