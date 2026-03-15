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
#include "lvgl.h"

/* Intra-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "clut.h"

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

/* Idea of this is to seperate lvgl driver specifics from gui_* files */
typedef lv_obj_t GuiScreen;

/**
 * @brief   Set the background color of a screen
 * @param   screen Pointer to the screen object to style
 * @param   screen_color entry to apply to the screen background
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_set_background_color(GuiScreen *screen, GuiColorId screen_color);

/**
 * @brief   Get the currently active LVGL screen
 * @return  Pointer to the active screen object
 */
GuiScreen *lvgl_get_active_screen();

/** @} */
