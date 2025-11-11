#pragma once

/************************************************************************************************
 * @file    button_manager.h
 *
 * @brief   Header file for managing and polling multiple buttons on the steering board, including
 *          drive modes, hazards, turn signals, cruise control, and regen.
 *
 * @date    2025-07-28
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "gpio.h"

/* Intra-component Headers */
#include "button.h"
#include "steering.h"

/**
 * @defgroup steering
 * @brief    steering Firmware
 * @{
 */

#define BUTTON_MANAGER_DEBUG 1 /**< Set to 1 to enable debug prints */

/**
 * @brief Button Manager structure
 */
typedef struct ButtonManager {
  Button buttons[NUM_STEERING_BUTTONS]; /**< Array of buttons */
} ButtonManager;

/**
 * @brief   Initialize the button manager
 * @param   storage Pointer to the SteeringStorage instance
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_INVALID_ARGS if an invalid parameter is passed in
 */
StatusCode button_manager_init(SteeringStorage *storage);

/**
 * @brief   Update the button manager
 * @param   manager Pointer to the ButtonManager instance
 * @return  STATUS_CODE_OK if updated successfully
 *          STATUS_CODE_UNINITIALIZED if the system has not already been initialized
 */
StatusCode button_manager_update(void);

/** @} */
