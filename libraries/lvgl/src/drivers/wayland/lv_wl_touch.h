#pragma once

/************************************************************************************************
 * @file    lv_wl_touch.h
 *
 * @brief   Lv Wl Touch
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
 * @defgroup lv_wl_touch
 * @brief    lv_wl_touch Firmware
 * @{
 */

#ifndef LV_WL_TOUCH_H
#define LV_WL_TOUCH_H

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

lv_indev_t * lv_wayland_touch_create(void);

/**
 * Get touchscreen input device for given LVGL display
 * @param display LVGL display
 * @return input device connected to touchscreen, or NULL on error
 */
lv_indev_t * lv_wayland_get_touchscreen(lv_display_t * display);

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_WAYLAND */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_WL_TOUCH_H*/

/** @} */
