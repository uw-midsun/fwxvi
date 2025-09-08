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

/**
 * @defgroup BUTTON_MANAGER
 * @brief High-level manager for polling and updating multiple button instances on the steering board.
 * @{
 */

#define BUTTON_MANAGER_DEBUG 0 /**< Set to 1 to enable debug prints */

#define BUTTON_MANAGER_MAX_BUTTONS 10 /**< Maximum number of buttons supported */

/**
 * @brief Button Manager structure
 */
typedef struct {
  Button buttons[BUTTON_MANAGER_MAX_BUTTONS]; /**< Array of buttons */
  uint8_t num_buttons;                        /**< Number of buttons in the array */
} ButtonManager;

/**
 * @brief   Initialize the button manager
 * @param   manager Pointer to the ButtonManager instance
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_INVALID_ARGS if an invalid parameter is passed in
 */

StatusCode button_manager_init(ButtonManager *manager);

/**
 * @brief   Update the button manager
 * @param   manager Pointer to the ButtonManager instance
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_INVALID_ARGS if an invalid parameter is passed in
 */
StatusCode button_manager_update(ButtonManager *manager);

/** @} */
