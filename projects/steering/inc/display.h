#pragma once

/************************************************************************************************
 * @file    display.h
 *
 * @brief   Header file for display control
 *
 * @date    2025-07-28
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "steering.h"

/**
 * @defgroup steering
 * @brief    steering Firmware
 * @{
 */

/**
 * @brief   Initialize the display
 * @param   storage Pointer to the SteeringStorage instance
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_INVALID_ARGS if an invalid parameter is passed in
 */
StatusCode display_init(SteeringStorage *storage);

StatusCode display_rx_slow();
StatusCode display_rx_medium();
StatusCode display_rx_fast();

/** @} */
