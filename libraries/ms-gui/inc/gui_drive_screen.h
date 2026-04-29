#pragma once

/************************************************************************************************
 * @file   gui_drive_screen.h
 *
 * @brief  High-level drive screen composition
 *
 * @date   2026-04-17
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "clut.h"
#include "lvgl_screens.h"

/**
 * @defgroup gui_drive_screen
 * @brief    gui_drive_screen Firmware
 * @{
 */

/**
 * @brief   Create the drive screen from common and drive-specific widgets
 * @param   screen Screen root to populate
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_drive_screen_init(GuiScreen *screen);

/**
 * @brief   Reset cached drive widget handles after their LVGL parent is deleted
 */
void gui_drive_screen_deinit(void);

/**
 * @brief   Update the speedometer needle
 * @param   speed_kmh Current speed in km/h
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_drive_screen_widget_set_speed(int16_t speed_kmh);

/**
 * @brief   Update the vertical throttle percentage bar
 * @param   percent Current percentage (0-100)
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_drive_screen_widget_set_throttle_bar(uint8_t percent);

/**
 * @brief   Update the vertical brake percentage bar
 * @param   percent Current percentage (0-100)
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_drive_screen_widget_set_brake_bar(uint8_t percent);

/**
 * @brief   Update the vertical brake percentage bar color
 * @param   color_id Color to apply to the brake bar fill
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_drive_screen_widget_set_brake_bar_color(GuiColorId color_id);

/**
 * @brief   Update the cruise control set speed
 * @param   cruise_control_speed_kmh The cruise control set speed in km/h
 * @param   is_cc_enabled Whether cruise control is enabled
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_drive_screen_widget_set_cc_speed(uint16_t cruise_control_speed_kmh, bool is_cc_enabled);

/**
 * @brief   Update the vertical brake percentage bar color
 * @param   color_id
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_widgets_set_brake_bar_color(GuiColorId color_id);

/** @} */
