#pragma once

/************************************************************************************************
 * @file    lv_nuttx_touchscreen.h
 *
 * @brief   Lv Nuttx Touchscreen
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../indev/lv_indev.h"

/* Intra-component Headers */

/**
 * @defgroup lv_nuttx_touchscreen
 * @brief    lv_nuttx_touchscreen Firmware
 * @{
 */

/*********************
 *      INCLUDES
 *********************/

#ifndef LV_NUTTX_TOUCHSCREEN_H
#define LV_NUTTX_TOUCHSCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_NUTTX

#if LV_USE_NUTTX_TOUCHSCREEN

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
 * Initialize indev with specified input device.
 * @param dev_path      path of input device
 */
lv_indev_t * lv_nuttx_touchscreen_create(const char * dev_path);

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_NUTTX_TOUCHSCREEN */

#endif /* LV_USE_NUTTX*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_NUTTX_TOUCHSCREEN_H */

/** @} */
