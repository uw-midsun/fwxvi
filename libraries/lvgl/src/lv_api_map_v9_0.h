#pragma once

/************************************************************************************************
 * @file    lv_api_map_v9_0.h
 *
 * @brief   Lv Api Map V9 0
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "misc/lv_types.h"

/* Intra-component Headers */

/**
 * @defgroup lv_api_map_v9_0
 * @brief    lv_api_map_v9_0 Firmware
 * @{
 */

#ifndef LV_API_MAP_V9_0_H
#define LV_API_MAP_V9_0_H

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

/**********************
 *      MACROS
 **********************/
#define lv_image_set_align               lv_image_set_inner_align
#define lv_image_get_align               lv_image_get_inner_align

#ifndef LV_DRAW_LAYER_SIMPLE_BUF_SIZE
#define LV_DRAW_LAYER_SIMPLE_BUF_SIZE    LV_DRAW_SW_LAYER_SIMPLE_BUF_SIZE
#endif

#define lv_button_bind_checked           lv_obj_bind_checked

#define LV_DRAW_BUF_DEFINE               LV_DRAW_BUF_DEFINE_STATIC

#define _lv_utils_bsearch                lv_utils_bsearch
#define lv_draw_buf_align_user           lv_draw_buf_align_ex
#define lv_draw_buf_create_user          lv_draw_buf_create_ex
#define lv_draw_buf_width_to_stride_user lv_draw_buf_width_to_stride_ex
#define lv_draw_buf_dup_user             lv_draw_buf_dup_ex

#define lv_draw_buf_invalidate_cache_user(handlers, drawbuf, area)  lv_draw_buf_invalidate_cache(drawbuf, area)
#define lv_draw_buf_flush_cache_user(handlers, drawbuf, area)       lv_draw_buf_flush_cache(drawbuf, area)
#define lv_draw_buf_destroy_user(handlers, drawbuf)                 lv_draw_buf_destroy(drawbuf)

/**********************
 *      MACROS
 **********************/

/**********************
 * DEPRECATED FUNCTIONS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_API_MAP_V9_0_H*/

/** @} */
