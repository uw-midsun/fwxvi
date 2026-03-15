#pragma once

/************************************************************************************************
 * @file    lv_libwebp.h
 *
 * @brief   Lv Libwebp
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_libwebp
 * @brief    lv_libwebp Firmware
 * @{
 */

#ifndef LV_LIBWEBP_H
#define LV_LIBWEBP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_LIBWEBP

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
 * Register the WEBP decoder functions in LVGL
 */
void lv_libwebp_init(void);

void lv_libwebp_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_LIBWEBP*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_LIBWEBP_H*/

/** @} */
