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

/* Intra-component Headers */
#include "status.h"

/**
 * @defgroup GUI
 * @brief    GUI Firmware
 * @{
 */

typedef lv_obj_t GuiScreen;

StatusCode lvgl_set_background_color(GuiScreen *screen, GuiColorId screen_color);
GuiScreen *lvgl_get_active_screen();

/** @} */
