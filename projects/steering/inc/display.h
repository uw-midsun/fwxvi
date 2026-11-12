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
#include "gpio.h"
#include "pwm.h"
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

// AP3032 datasheet mentions a signal frequency of >=25 kHz to avoid audible noise.
// After trying the code on the board, looks like 50 kHz minimizes audible noise while
// providing decent resolution on the brightness.
#define BACKLIGHT_FREQ_HZ 50000
#define BACKLIGHT_DEFAULT_BRIGHTNESS 100U

#define BACKLIGHT_PWM_TIMER PWM_TIMER_15
#define BACKLIGHT_PWM_CHANNEL PWM_CHANNEL_1
#define BACKLIGHT_GPIO_AF GPIO_ALT14_TIM15

/**
 * @brief   Initialize the display
 * @param   storage Pointer to the SteeringStorage instance
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_INVALID_ARGS if an invalid parameter is passed in
 */
StatusCode display_init(SteeringStorage *storage);

/**
 * @brief   Initialize the display's backlight. This is run as a part of display_init
 * @param   storage Pointer to the SteeringStorage instance
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_INVALID_ARGS if an invalid parameter is passed in
 */
StatusCode display_backlight_init(SteeringStorage *storage);

/**
 * @brief Adjust the brightness of the display
 * @param percentage Percentage to set the display brightness to, between 0 to 100
 * @param persist Whether the brightness should be persisted in flash memory
 * @return STATUS_CODE_OK if completed successfully
 *         STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect, or
 *          the method is run before the display backlight is initialized
 */
StatusCode display_set_brightness(uint16_t percentage, bool persist);

StatusCode display_rx_slow();
StatusCode display_rx_medium();
StatusCode display_rx_fast();
StatusCode display_run();

/** @} */
