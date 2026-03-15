#pragma once

/************************************************************************************************
 * @file    lv_libpng.h
 *
 * @brief   Lv Libpng
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_libpng
 * @brief    lv_libpng Firmware
 * @{
 */

#ifndef LV_LIBPNG_H
#define LV_LIBPNG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_LIBPNG

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
 * Register the PNG decoder functions in LVGL
 */
void lv_libpng_init(void);

void lv_libpng_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_LIBPNG*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_LIBPNG_H*/

/** @} */
