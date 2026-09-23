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

// TODO: Verify backlight PWM period. I couldn't find the chip myself in the schematics,
// but the previous PR mentioned that the backlight driver was the AP3032, and its datasheet
// suggests a minimum PWM duty cycle of 25kHz
#define DISPLAY_BACKLIGHT_PERIOD_US 20
#define DISPLAY_BACKLIGHT_DEFAULT_DUTY_CYCLE 100

/**
 * @brief   Initialize the display
 * @param   storage Pointer to the SteeringStorage instance
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_INVALID_ARGS if an invalid parameter is passed in
 */
StatusCode display_init(SteeringStorage *storage);

/**
 * @brief Adjust the brightness of the display
 * @param percentage Percentage to set the display brightness to, between 0 to 100
 * @return STATUS_CODE_OK if completed successfully
 *         STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect, or
 *          the method is run before the display is initialized
 */
StatusCode display_set_brightness(uint16_t percentage);

StatusCode display_rx_slow();
StatusCode display_rx_medium();
StatusCode display_rx_fast();
StatusCode display_run();

/** @} */
