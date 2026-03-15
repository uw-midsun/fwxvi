#pragma once

/************************************************************************************************
 * @file    lv_cache_lru_rb.h
 *
 * @brief   Lv Cache Lru Rb
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../lv_cache_private.h"

/* Intra-component Headers */

/**
 * @defgroup lv_cache_lru_rb
 * @brief    lv_cache_lru_rb Firmware
 * @{
 */

#ifndef LV_CACHE_LRU_RB_H
#define LV_CACHE_LRU_RB_H

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

/*************************
 *    GLOBAL VARIABLES
 *************************/
LV_ATTRIBUTE_EXTERN_DATA extern const lv_cache_class_t lv_cache_class_lru_rb_count;
LV_ATTRIBUTE_EXTERN_DATA extern const lv_cache_class_t lv_cache_class_lru_rb_size;
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CACHE_LRU_RB_H*/

/** @} */
