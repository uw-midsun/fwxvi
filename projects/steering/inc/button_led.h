#pragma once

/************************************************************************************************
 * @file    button_led.h
 *
 * @brief   Header file for button led control
 *
 * @date    2025-01-28
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "button.h"
#include "button_led_manager.h"

/**
 * @defgroup button_led
 * @brief    button_led Firmware
 * @{
 */

/**
 * @brief   Set an LED to its assigned color
 */
StatusCode button_led_enable(SteeringButtons button);

/**
 * @brief   Turn a given LED off
 */
StatusCode button_led_disable(SteeringButtons button);

/**
 * @brief   Toggle a given LED between it's assigned color and off
 */
StatusCode button_led_toggle(SteeringButtons button);

/** @} */
