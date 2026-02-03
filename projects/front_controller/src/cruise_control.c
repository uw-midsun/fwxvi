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
   * 1.1. if eP < min_treshold, set velocity directly
   * 2. calculate eI, eD
   * 2.1. bound eI if needed
   * 3. calculate result kP * eP + ...
   * 4. compare result to previous set_motor_velocity -> acceleration limiting
   *
   * Notes: eI must be reset on any pedal action
   */

  float e_p = s_cruise_control_storage.current_motor_velocity - s_cruise_control_storage.target_motor_velocity;

  // If we are within a minimum threshold, write the target velocity directly and let the WS22 take over
  if (fabsf(e_p) < CC_MIN_THRESHOLD) {
    s_cruise_control_storage.set_motor_velocity = s_cruise_control_storage.target_motor_velocity;
    return STATUS_CODE_OK;
  }

  float res = CC_KP * e_p;

  // Clamp under acceleration limit
  res = fminf(res, ACCELERATION_LIMIT_PER_1000HZ_CYCLE);

  // Store set velocity in cruise control storage
  s_cruise_control_storage.set_motor_velocity = s_cruise_control_storage.current_motor_velocity + res;

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
