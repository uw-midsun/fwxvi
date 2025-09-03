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

/**
 * @defgroup BUTTON_MANAGER
 * @brief High-level manager for polling and updating multiple button instances on the steering board.
 * @{
 */

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "gpio.h"

/* Intra-component Headers */
#include "button.h"

#define BUTTON_MANAGER_MAX_BUTTONS 10 /**< Maximum number of buttons supported */

/**
 * @brief Button Manager structure
 */
typedef struct {
  Button buttons[BUTTON_MANAGER_MAX_BUTTONS]; /**< Array of buttons */
  uint8_t num_buttons;                        /**< Number of buttons in the array */
} ButtonManager;

/**
 * @brief Initialize the button manager
 *
 * @param manager Pointer to the ButtonManager instance
 */

void button_manager_init(ButtonManager *manager, ButtonConfig *configs, uint8_t num_buttons);

/**
 * @brief Update the button manager
 *
 * @param manager Pointer to the ButtonManager instance
 */
void button_manager_update(ButtonManager *manager);

/** @} */
