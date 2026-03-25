#pragma once

/************************************************************************************************
 * @file   motor_can.h
 *
 * @brief  Header file for motor can
 *
 * @date   2025-11-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "front_controller.h"
#include "ws22_motor_can.h"

/**
 * @defgroup Front_Controller
 * @brief    Front Controller Board Firmware
 * @{
 */

#define VEL_TO_RPM_RATIO 0.57147  // from MSXV

/**
 * @brief   Updates target current and velocity of motors based on drive state
 * @return  STATUS_CODE_OK if values are set successfully
 *          STATUS_CODE_INVALID_ARGS if drive state is invalid
 *          STATUS_CODE_UNINITIALIZED if motor can is uninitialized
 */
StatusCode motor_can_update_target_current_velocity();

/**
 * @brief   Initializes the motor can, loads front controller storage data
 *
 * @return  STATUS_CODE_OK if front controller is initialized successfully
 *          STATUS_CODE_INVALID_ARGS if storage pointer is invalid
 */
StatusCode motor_can_init(FrontControllerStorage *storage);

/**
 * @brief retrieve current vehicle drive state
 */
StatusCode motor_can_get_current_state(VehicleDriveState *current_state);

/**
 * @brief print a state as a string
 */
char *motor_can_get_current_state_str();

/** @} */
