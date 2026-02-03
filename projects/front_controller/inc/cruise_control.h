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

#define CC_MIN_THRESHOLD 1
#define CC_KP 0.001
#define ACCELERATION_LIMIT_M_S2 1
#define ACCELERATION_LIMIT_KM_HS ACCELERATION_LIMIT_M_S2 * 3.6
#define ACCELERATION_LIMIT_PER_1000HZ_CYCLE ACCELERATION_LIMIT_KM_HS / 1000

typedef struct CruiseControlStorage {
  float current_motor_velocity;
  float target_motor_velocity;
  float set_motor_velocity;

} CruiseControlStorage;

/**
 * @brief   Runs one cruise control calculation cycle. Called within motor_can.c when cruise control mode is on
 * @returns STATUS_CODE_OK if successful
 *          STATUS_CODE_INVALID_ARGS if cruise control is not initialized
 */
StatusCode cruise_control_run();

/**
 * @brief   Initializes the cruise control module
 * @returns STATUS_CODE_OK if initialization is successful
 *          STATUS_CODE_INVALID_ARGS if storage is null
 */
StatusCode cruise_control_init(FrontControllerStorage *storage);

/** @} */
