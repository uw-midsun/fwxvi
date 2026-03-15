#pragma once

/************************************************************************************************
 * @file    lv_wayland.h
 *
 * @brief   Lv Wayland
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"
#include "lv_wl_keyboard.h"
#include "lv_wl_pointer.h"
#include "lv_wl_pointer_axis.h"
#include "lv_wl_touch.h"
#include "lv_wl_window.h"

/* Intra-component Headers */

/**
 * @defgroup lv_wayland
 * @brief    lv_wayland Firmware
 * @{
 */

#ifndef LV_WAYLAND_H
#define LV_WAYLAND_H

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

/**
 * Wrapper around lv_timer_handler
 * @note Must be called in the application run loop instead of the
 * regular lv_timer_handler provided by LVGL
 * @return time till it needs to be run next (in ms)
 */
uint32_t lv_wayland_timer_handler(void);

/**
 * Retrieves the file descriptor of the wayland socket
 */
int lv_wayland_get_fd(void);

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_WAYLAND */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_WAYLAND_H */

/** @} */
