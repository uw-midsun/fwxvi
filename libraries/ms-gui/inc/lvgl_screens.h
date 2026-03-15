#pragma once

/************************************************************************************************
 * @file   lvgl_screens.h
 *
 * @brief  Wrapper around LVGL screens
 *
 * @date   2026-03-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "clut.h"
#include "lvgl.h"
#include "status.h"

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

typedef lv_obj_t GuiScreen;

/**
 * @brief   Set the background color for a screen
 * @param   screen Pointer to the target screen
 * @param   screen_color Semantic GUI color to apply as the screen background
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_set_background_color(GuiScreen *screen, GuiColorId screen_color);

/**
 * @brief   Get the currently active LVGL screen
 * @return  Pointer to the active screen, or NULL on failure
 */
GuiScreen *lvgl_get_active_screen();

/** @} */
