#pragma once

/************************************************************************************************
 * @file    lv_lodepng.h
 *
 * @brief   Lv Lodepng
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_lodepng
 * @brief    lv_lodepng Firmware
 * @{
 */

#ifndef LV_LODEPNG_H
#define LV_LODEPNG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_LODEPNG

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
void lv_lodepng_init(void);

void lv_lodepng_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_LODEPNG*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_LODEPNG_H*/

/** @} */
