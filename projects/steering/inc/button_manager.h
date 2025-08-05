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