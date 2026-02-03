#pragma once

/************************************************************************************************
 * @file   cruise_control.h
 *
 * @brief  Header file for front controller cruise control handling
 *
 * @date   2026-02-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "front_controller.h"

/**
 * @defgroup Front_Controller
 * @brief    Front Controller Board Firmware
 * @{
 */

typedef struct AccelPedalStorage {
  float current_motor_velocity;
  float target_motor_velocity;
  float set_motor_velocity;

} CruiseControlStorage;


StatusCode cruise_control_run();

StatusCode cruise_control_init(FrontControllerStorage *storage);

/** @} */
