#pragma once

/************************************************************************************************
 * @file   gui_widgets.h
 *
 * @brief  High-level LVGL widget abstractions for the vehicle display
 *
 * @date   2026-03-10
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

/**
 * @brief   Initialize the widget layer and create the main screen
 * @details Sets up common LVGL styles, creates the speedometer (round scale)
 *          and vertical percentage bar. Must be called after lvgl_driver_init().
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_init(void);

/**
 * @brief   Update the speedometer needle
 * @param   speed_kmh Current speed in km/h
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_speed(int16_t speed_kmh);

/**
 * @brief   Update the vertical throttle percentage bar
 * @param   percent Current percentage (0-100)
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_throttle_bar(uint8_t percent);

/**
 * @brief   Update the vertical brake percentage bar
 * @param   percent Current percentage (0-100)
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_brake_bar(uint8_t percent);

/**
 * @brief   Update the horizontal segmented soc bar
 * @param   soc_percent Current percentage (0-100)
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_soc_bar(uint8_t soc_percent);

/** @} */
