#pragma once

/************************************************************************************************
 * @file   pedal_calib.h
 *
 * @brief  Steering-side pedal calibration handler
 *
 * @date   2026-04-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "steering.h"

/**
 * @defgroup Steering
 * @brief    Steering Firmware
 * @{
 */

/**
 * @brief   Initialize pedal calibration module
 * @param   storage Pointer to steering storage
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode steering_pedal_calib_init(SteeringStorage *storage);

/**
 * @brief   Start pedal calibration via CAN
 * @param   storage Pointer to steering storage
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode steering_pedal_calib_start(SteeringStorage *storage);

/**
 * @brief   Handle CAN status updates from front controller during calibration
 * @param   storage Pointer to steering storage
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode steering_pedal_calib_rx(SteeringStorage *storage);

/**
 * @brief   Check if calibration is currently active
 * @param   storage Pointer to steering storage
 * @return  true if calibration is active, false otherwise
 */
bool steering_pedal_calib_is_active(SteeringStorage *storage);

/** @} */
