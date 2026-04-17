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
#include "display_defs.h"
#include "status.h"

/* Intra-component Headers */
#include "steering.h"

/**
 * @defgroup steering
 * @brief    steering Firmware
 * @{
 */

/**
 * @brief LTDC Timing config
 * From: https://www.buydisplay.com/download/ic/ST7282.pdf
 * Specifically 10.1.1 Parallel 24-bit RGB Timing Table
 */
#define HORIZONTAL_SYNC_WIDTH 4
#define VERTICAL_SYNC_WIDTH 4
#define HORIZONTAL_BACK_PORCH 43
#define VERTICAL_BACK_PORCH 12
#define HORIZONTAL_FRONT_PORCH 8
#define VERTICAL_FRONT_PORCH 8

/**
 * @brief   Initialize the display
 * @param   storage Pointer to the SteeringStorage instance
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_INVALID_ARGS if an invalid parameter is passed in
 */
StatusCode display_init(SteeringStorage *storage);

/**
 * @brief   Set the display backlight brightness
 * @details This function sets the duty cycle of the PWM signal driving the AP3032 backlight driver's PWM/CTRL pin.
 *          The duty cycle modulates the average current, and therefore the perceived brightness of the display backlight.
 *          This function does not change the peak current, only its duty cycle.
 * @param   percentage  Brightness level as a percentage (0–100)
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_INVALID_ARGS if an invalid parameter is passed into pwm_set_dc
 */
StatusCode display_set_brightness(uint8_t percentage);

StatusCode display_rx_slow();
StatusCode display_rx_medium();
StatusCode display_rx_fast();
StatusCode display_run();

/** @} */
