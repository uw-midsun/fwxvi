#pragma once

/************************************************************************************************
 * @file   gui_display_brightness_screen.h
 *
 * @brief  High-level display brightness screen composition
 *
 * @date   2026-11-10
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "lvgl_screens.h"

/**
 * @defgroup gui_display_brightness_screen
 * @brief    gui_display_brightness_screen Firmware
 * @{
 */

/**
 * @brief   Create the display brightness control screen (title + bar)
 * @param   screen Screen root to populate
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_display_brightness_screen_init(GuiScreen *screen);

/**
 * @brief   Reset cached display brightness screen widget handles after their LVGL parent is deleted
 */
void gui_display_brightness_screen_deinit(void);

/**
 * @brief   Update the horizontal brightness percentage bar
 * @param   percent Current percentage (0-100)
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_display_brightness_screen_widget_set_bar(uint8_t percent);

/** @} */
