#pragma once

/************************************************************************************************
 * @file    lv_wl_pointer.h
 *
 * @brief   Lv Wl Pointer
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
 * @defgroup lv_wl_pointer
 * @brief    lv_wl_pointer Firmware
 * @{
 */

#ifndef LV_WL_POINTER_H
#define LV_WL_POINTER_H

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

lv_indev_t * lv_wayland_pointer_create(void);

/**
 * Obtains the input device of the mouse pointer
 * @note It is used to create an input group on application start
 * @param disp Reference to the LVGL display associated to the window
 * @return The input device
 */
lv_indev_t * lv_wayland_get_pointer(lv_display_t * disp);

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_WAYLAND */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_WL_POINTER_H*/

/** @} */
