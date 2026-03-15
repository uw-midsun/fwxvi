#pragma once

/************************************************************************************************
 * @file    lv_libjpeg_turbo.h
 *
 * @brief   Lv Libjpeg Turbo
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_libjpeg_turbo
 * @brief    lv_libjpeg_turbo Firmware
 * @{
 */

#ifndef LV_LIBJPEG_TURBO_H
#define LV_LIBJPEG_TURBO_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_LIBJPEG_TURBO

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
 * Register the JPEG-Turbo decoder functions in LVGL
 */
void lv_libjpeg_turbo_init(void);

void lv_libjpeg_turbo_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_LIBJPEG_TURBO*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_LIBJPEG_TURBO_H*/

/** @} */
