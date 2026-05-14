#pragma once

/************************************************************************************************
 * @file   gui_pedal_calib_screen.h
 *
 * @brief  High-level pedal calibration screen
 *
 * @date   2026-04-27
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
 * @defgroup gui_pedal_calib_screen
 * @brief    gui_pedal_calib_screen Firmware
 * @{
 */

/**
 * @brief   Create the pedal calibration screen
 * @param   screen Screen root to populate
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_pedal_calib_screen_init(GuiScreen *screen);

/**
 * @brief   Reset cached pedal calib widget handles after their LVGL parent is deleted
 */
void gui_pedal_calib_screen_deinit(void);

/**
 * @brief   Update the large display text with calibration status
 * @param   text Text to display (e.g., "Press fully", "Release fully")
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode gui_pedal_calib_widget_big_text(const char *text);

/** @} */
