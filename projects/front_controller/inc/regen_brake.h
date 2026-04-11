#pragma once

/************************************************************************************************
 * @file   regen_brake.h
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

/**
 * @brief   Runs one regen brake calculation cycle. Called within motor_can.c when in regen brake state
 * @returns STATUS_CODE_OK if successful
 *          STATUS_CODE_INVALID_ARGS if regen_brake is not initialized
 */
StatusCode regen_brake_run(float *target_current, bool *direction);

/**
 * @brief   Initializes the regen brake module
 * @returns STATUS_CODE_OK if initialization is successful
 *          STATUS_CODE_INVALID_ARGS if storage is null
 */
StatusCode regen_brake_init(FrontControllerStorage *storage);

/** @} */
