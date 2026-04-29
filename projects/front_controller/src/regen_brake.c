/************************************************************************************************
 * @file    regen_brake.c
 *
 * @brief   Source file for front controller regen brake calculation handling
 *
 * @date    2026-04-10
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <math.h>

/* Inter-component Headers */
#include "log.h"

/* Intra-component Headers */
#include "cruise_control.h"
#include "front_controller.h"
#include "front_controller_getters.h"
#include "ws22_motor_can.h"

static FrontControllerStorage *front_controller_storage;

static float current_velocity;
static float regen_percentage;

#define REGEN_BRAKE_DEBUG 0U

#if (REGEN_BRAKE_DEBUG == 1)
#define CONDITIONAL_LOG_DEBUG(...) LOG_DEBUG(__VA_ARGS__)
#else
#define CONDITIONAL_LOG_DEBUG(...) \
  do {                             \
  } while (0)
#endif

#define REGEN_BRAKE_STOPPING_THRESHOLD 10U

StatusCode regen_brake_run(float *target_current, bool *direction) {
  if (front_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  current_velocity = front_controller_storage->ws22_motor_can_storage->telemetry.motor_velocity;

  if (current_velocity > 0) {
    *direction = false;
  } else {
    *direction = true;
  }

  if (fabsf(current_velocity) < REGEN_BRAKE_STOPPING_THRESHOLD) {
    *target_current = 0;
    return STATUS_CODE_OK;
  }

  regen_percentage = (front_controller_storage->brake_percentage - front_controller_storage->config->brake_pedal_deadzone) / (1.0f - front_controller_storage->config->brake_pedal_deadzone);

  // y = 2x - x^2 to curve the value upwards
  *target_current = 2 * regen_percentage - (regen_percentage * regen_percentage);

  return STATUS_CODE_OK;
}

StatusCode regen_brake_init(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  front_controller_storage = storage;
  return STATUS_CODE_OK;
}
