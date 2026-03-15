#pragma once

/************************************************************************************************
 * @file    lv_qnx.h
 *
 * @brief   LVGL driver for the QNX Screen compositing window manager
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <screen/screen.h>
#include <stdbool.h>

/* Inter-component Headers */
#include "../../display/lv_display.h"
#include "../../indev/lv_indev.h"

/* Intra-component Headers */

/**
 * @defgroup lv_qnx
 * @brief    lv_qnx Firmware
 * @{
 */

#ifndef LV_QNX_H
#define LV_QNX_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_QNX

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a window to use as a display for LVGL.
 * @param   hor_res     The horizontal resolution (size) of the window
 * @param   ver_res     The vertical resolution (size) of the window
 * @return  A pointer to a new display object if successful, NULL otherwise
 */
lv_display_t * lv_qnx_window_create(int32_t hor_res, int32_t ver_res);

/**
 * Set the title of the window identified by the given display.
 * @param   disp    The display object for the window
 * @param   title   The new title to set
 */
void lv_qnx_window_set_title(lv_display_t * disp, const char * title);

/**
 * Create a pointer input device for the display.
 * Only one pointer object is currently supported.
 * @param   disp    The display object associated with the device
 * @return  true if successful, false otherwise
 */
bool lv_qnx_add_pointer_device(lv_display_t * disp);

/**
 * Create a keyboard input device for the display.
 * Only one keyboard object is currently supported.
 * @param   disp    The display object associated with the device
 * @return  true if successful, false otherwise
 */
bool lv_qnx_add_keyboard_device(lv_display_t * disp);

/**
 * Runs the event loop for the display.
 * The function only returns in response to a close event.
 * @param   disp    The display for the event loop
 * @return  Exit code
 */
int lv_qnx_event_loop(lv_display_t * disp);

/**********************
 *      MACROS
 **********************/

#endif /* LV_DRV_QNX */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_QNX_H */

/** @} */
