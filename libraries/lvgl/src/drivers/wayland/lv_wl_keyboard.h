#pragma once

/************************************************************************************************
 * @file    lv_wl_keyboard.h
 *
 * @brief   Lv Wl Keyboard
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../indev/lv_indev.h"
#include "../../indev/lv_indev_gesture.h"

/* Intra-component Headers */

/**
 * @defgroup lv_wl_keyboard
 * @brief    lv_wl_keyboard Firmware
 * @{
 */

#ifndef LV_WL_KEYBOARD_H
#define LV_WL_KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_WAYLAND

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_indev_t * lv_wayland_keyboard_create(void);

/**
 * Get keyboard input device for given LVGL display
 * @param display LVGL display
 * @return input device connected to keyboard, or NULL on error
 */
lv_indev_t * lv_wayland_get_keyboard(lv_display_t * display);

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_WAYLAND */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_WL_KEYBOARD_H*/

/** @} */
