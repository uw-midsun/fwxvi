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

StatusCode cruise_control_init(SteeringStorage *steering_storage);

StatusCode cruise_control_down_handler();

StatusCode cruise_control_up_handler();

StatusCode cruise_control_run();


/** @} */
