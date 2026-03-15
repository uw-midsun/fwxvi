#pragma once

/************************************************************************************************
 * @file    lv_draw_vg_lite_type.h
 *
 * @brief   Lv Draw Vg Lite Type
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <vg_lite.h>

/* Inter-component Headers */
#include "../../debugging/vg_lite_tvg/vg_lite.h"
#include "../../libs/vg_lite_driver/inc/vg_lite.h"
#include "../../lv_conf_internal.h"
#include "../../misc/lv_array.h"
#include "../lv_draw_private.h"

/* Intra-component Headers */

/**
 * @defgroup lv_draw_vg_lite_type
 * @brief    lv_draw_vg_lite_type Firmware
 * @{
 */

#ifndef LV_DRAW_VG_LITE_TYPE_H
#define LV_DRAW_VG_LITE_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_DRAW_VG_LITE

#if LV_USE_VG_LITE_THORVG
#else
#if LV_USE_VG_LITE_DRIVER
#else
#endif
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

struct _lv_vg_lite_pending_t;
struct _lv_vg_lite_grad_ctx_t;

typedef struct _lv_draw_vg_lite_unit_t {
    lv_draw_unit_t base_unit;
    lv_draw_task_t * task_act;
    lv_area_t current_scissor_area;

    struct _lv_vg_lite_pending_t * image_dsc_pending;

    struct _lv_vg_lite_grad_ctx_t * grad_ctx;

    lv_cache_t * stroke_cache;

    lv_cache_t * bitmap_font_cache;
    struct _lv_vg_lite_pending_t * bitmap_font_pending;
    struct _lv_vg_lite_pending_t * letter_pending;

    uint16_t flush_count;
    uint16_t letter_count;
    vg_lite_buffer_t target_buffer;
    vg_lite_matrix_t global_matrix;
    struct _lv_vg_lite_path_t * global_path;
    bool path_in_use;
} lv_draw_vg_lite_unit_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_VG_LITE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_VG_LITE_TYPE_H*/

/** @} */
