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
#include "front_controller_getters"

static FrontControllerStorage *front_controller_storage = NULL;
static CruiseControlStorage s_cruise_control_storage = { 0 };

StatusCode cruise_control_run() {
  if (front_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  return STATUS_CODE_OK;
}

StatusCode cruise_control_set_target_velocity(float new_target_velocity) {}

StatusCode cruise_control_init(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  front_controller_storage = storage;
  s_cruise_control_storage = storage->cruise_control_storage;
  return STATUS_CODE_OK;
}
