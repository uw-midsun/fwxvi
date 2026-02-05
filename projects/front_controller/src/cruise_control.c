/************************************************************************************************
 * @file    cruise_control.c
 *
 * @brief   Source file for front controller cruise control handling
 *
 * @date    2026-02-03
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <math.h>

/* Inter-component Headers */
#include "adc.h"
#include "gpio.h"
#include "log.h"

/* Intra-component Headers */
#include "cruise_control.h"
#include "front_controller.h"
#include "front_controller_getters.h"
#include "ws22_motor_can.h"

static FrontControllerStorage *front_controller_storage;
static CruiseControlStorage s_cruise_control_storage = { 0 };

#define DT 0.001  // cruise_control_run() is a 1000Hz task

StatusCode cruise_control_run() {
  if (front_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  s_cruise_control_storage.current_motor_velocity = front_controller_storage->vehicle_speed_kph;

  /**
   * Perform PID calculations on current and target motor velocity...
   *
   *
   * 1. calculate eP
   * 1.1. if eP < min_threshold, set current directly
   * 2. calculate eI, eD
   * 2.1. bound eI if needed
   * 3. calculate result kP * eP + ...
   * 4. compare result to previous set_motor_velocity -> acceleration limiting
   *
   * Notes: eI must be reset on any pedal action
   */

  float e_p = s_cruise_control_storage.target_motor_velocity - s_cruise_control_storage.current_motor_velocity;

  // If we are within a minimum threshold, stop increasing the set current
  if (fabsf(e_p) < CC_MIN_THRESHOLD) {
    LOG_DEBUG("e_p within CC min threshold\r\n");
    // Set current should not change
    return STATUS_CODE_OK;
  }

  float res = CC_KP * e_p;

  // Clamp under current step limit
  res = fminf(res, CURRENT_STEP_LIMIT);

  // Store set velocity in cruise control storage
  s_cruise_control_storage.set_current += res;

  LOG_DEBUG("MotVel: %d | TarVel: %d | res: %dx10^-6 | curr: %dmA\r\n", (int16_t)s_cruise_control_storage.current_motor_velocity, (int16_t)s_cruise_control_storage.target_motor_velocity,
            (int16_t)(res * 1000000), (int16_t)(front_controller_storage->ws22_motor_can_storage->control.current * 1000));

  return STATUS_CODE_OK;
}

StatusCode cruise_control_init(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  front_controller_storage = storage;
  front_controller_storage->cruise_control_storage = &s_cruise_control_storage;
  return STATUS_CODE_OK;
}
