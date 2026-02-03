#pragma once

/************************************************************************************************
 * @file    cruise_control.h
 *
 * @brief   Cruise Control
 *
 * @date    2025-11-12
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup cruise_control
 * @brief    cruise_control Firmware
 * @{
 */

/**
 * @brief   Initailize cruise control
 * @param   storage Pointer to the SteeringStorage instance
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_INVALID_ARGS if an invalid parameter is passed in
 */
StatusCode cruise_control_init(SteeringStorage *steering_storage);

/**
 * @brief   Cruise control traget speed decrease event handler
 * @return  STATUS_CODE_OK if handled successfully
 *          STATUS_CODE_UNINITIALIZED if cruise control is not initialized
 */
StatusCode cruise_control_down_handler();

/**
 * @brief   Cruise control target speed increase event handler
 * @return  STATUS_CODE_OK if handled successfully
 *          STATUS_CODE_UNINITIALIZED if cruise control is not initialized
 */
StatusCode cruise_control_up_handler();

/**
 * @brief   Listen for cruise control buttons and update state appropriately
 * @return  STATUS_CODE_INVALID_ARGS if the car is not in drive or is not driving within the defined cruise control range
 */
StatusCode cruise_control_run_medium_cycle();

/** @} */
