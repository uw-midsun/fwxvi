#pragma once

/************************************************************************************************
 * @file    party_mode.h
 *
 * @brief   Header file for steering party mode
 *
 * @date    2025-07-28
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "gpio.h"
#include "status.h"

/* Intra-component Headers */
#include "steering.h"

/**
 * @defgroup steering
 * @brief    steering Firmware
 * @{
 */

/**
 * @brief   Initialize party mode system
 * @param   storage Pointer to the steering storage
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_INVALID_ARGS when params are invalid
 */
StatusCode party_mode_init(SteeringStorage *storage);

/**
 * @brief   Run party mode
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_UNINITIALIZED when party mode is not initialized
 */
StatusCode party_mode_run(void);

/**
 * @brief   Check if party mode is active
 * @return  TRUE when party mode is active
 *          FALSE when party mode is inactive
 */
bool party_mode_active(void);

/** @} */
