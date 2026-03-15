#pragma once

/************************************************************************************************
 * @file    lv_nuttx_image_cache.h
 *
 * @brief   Lv Nuttx Image Cache
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"

/* Intra-component Headers */
#include LV_STDBOOL_INCLUDE

/**
 * @defgroup lv_nuttx_image_cache
 * @brief    lv_nuttx_image_cache Firmware
 * @{
 */

#ifndef LV_NUTTX_IMAGE_CACHE_H
#define LV_NUTTX_IMAGE_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_nuttx_image_cache_init(bool use_independent_image_heap);

void lv_nuttx_image_cache_deinit(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_NUTTX_IMAGE_CACHE_H*/

/** @} */
