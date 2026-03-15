#pragma once

/************************************************************************************************
 * @file    lv_linux_fbdev.h
 *
 * @brief   Lv Linux Fbdev
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../../display/lv_display.h"

/* Intra-component Headers */

/**
 * @defgroup lv_linux_fbdev
 * @brief    lv_linux_fbdev Firmware
 * @{
 */

#ifndef LV_LINUX_FBDEV_H
#define LV_LINUX_FBDEV_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_LINUX_FBDEV

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
lv_display_t * lv_linux_fbdev_create(void);

lv_result_t lv_linux_fbdev_set_file(lv_display_t * disp, const char * file);

/**
 * Force the display to be refreshed on every change.
 * Expected to be used with LV_DISPLAY_RENDER_MODE_DIRECT or LV_DISPLAY_RENDER_MODE_FULL.
 */
void lv_linux_fbdev_set_force_refresh(lv_display_t * disp, bool enabled);

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_LINUX_FBDEV */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_LINUX_FBDEV_H */

/** @} */
