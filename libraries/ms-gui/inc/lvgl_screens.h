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
#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
#include "lvgl.h"
#endif

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
#if defined(STM32L4P5xx) || defined(MS_PLATFORM_X86)
typedef lv_obj_t GuiScreen;
#else
typedef void GuiScreen;
#endif

/**
 * @brief   Set the background color of a screen
 * @param   screen Pointer to the screen object to style
 * @param   screen_color entry to apply to the screen background
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_set_background_color(GuiScreen *screen, GuiColorId screen_color);

/**
 * @brief   Create a new LVGL screen root object
 * @param   screen Output pointer for the new screen root
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_screens_create(GuiScreen **screen);

/**
 * @brief   Load a screen as the active LVGL screen
 * @param   screen Screen root to make active
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_screens_load(GuiScreen *screen);

/**
 * @brief   Destroy a previously created screen root object
 * @param   screen Screen root to destroy
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode lvgl_screens_destroy(GuiScreen *screen);

/**
 * @brief   Get the currently active LVGL screen
 * @return  Pointer to the active screen object
 */
GuiScreen *lvgl_get_active_screen();

/**
 * @brief   Get the display top layer used for overlays
 * @return  Pointer to the LVGL top layer object
 */
GuiScreen *lvgl_get_top_layer();

/** @} */
