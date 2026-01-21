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

#ifdef STM32L4P5xx         /* Framebuffer takes up too much RAM on other STMs otherwise*/
#define DISPLAY_WIDTH 480  /**< Width of the display */
#define DISPLAY_HEIGHT 272 /**< Height of the display */
#else
#define DISPLAY_WIDTH 1  /**< Width of the display */
#define DISPLAY_HEIGHT 1 /**< Height of the display */
#endif

/**
 * @brief   Initialize the display
 * @param   storage Pointer to the SteeringStorage instance
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_INVALID_ARGS if an invalid parameter is passed in
 */
StatusCode display_init(SteeringStorage *storage);

/** @} */
