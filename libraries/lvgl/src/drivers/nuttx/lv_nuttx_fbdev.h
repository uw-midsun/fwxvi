#pragma once

/************************************************************************************************
 * @file    lv_nuttx_fbdev.h
 *
 * @brief   Lv Nuttx Fbdev
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../display/lv_display.h"

/* Intra-component Headers */

/**
 * @defgroup lv_nuttx_fbdev
 * @brief    lv_nuttx_fbdev Firmware
 * @{
 */

#ifndef LV_NUTTX_FBDEV_H
#define LV_NUTTX_FBDEV_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_NUTTX

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
 * Create a new display with NuttX backend.
 */
lv_display_t * lv_nuttx_fbdev_create(void);

/**
 * Initialize display with specified framebuffer device
 * @param disp      pointer to display with NuttX backend
 * @param file      the name of framebuffer device
 */
int lv_nuttx_fbdev_set_file(lv_display_t * disp, const char * file);

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_NUTTX */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_NUTTX_FBDEV_H */

/** @} */
