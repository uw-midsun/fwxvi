#pragma once

/************************************************************************************************
 * @file    lv_nuttx_cache.h
 *
 * @brief   Lv Nuttx Cache
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup lv_nuttx_cache
 * @brief    lv_nuttx_cache Firmware
 * @{
 */

#ifndef LV_NUTTX_CACHE_H
#define LV_NUTTX_CACHE_H

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

void lv_nuttx_cache_init(void);

void lv_nuttx_cache_deinit(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_NUTTX_CACHE_H*/

/** @} */
