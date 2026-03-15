#pragma once

/************************************************************************************************
 * @file    lv_draw_nema_gfx.h
 *
 * @brief   Lv Draw Nema Gfx
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"
#include "../../misc/lv_area_private.h"
#include "../lv_draw_arc.h"
#include "../lv_draw_buf_private.h"
#include "../lv_draw_image_private.h"
#include "../lv_draw_label_private.h"
#include "../lv_draw_line.h"
#include "../lv_draw_mask.h"
#include "../lv_draw_private.h"
#include "../lv_draw_rect_private.h"
#include "../lv_draw_triangle_private.h"
#include "../lv_draw_vector_private.h"
#include "../lv_image_decoder_private.h"
#include "lv_draw_nema_gfx_utils.h"

/* Intra-component Headers */

/**
 * @defgroup lv_draw_nema_gfx
 * @brief    lv_draw_nema_gfx Firmware
 * @{
 */

#ifndef LV_DRAW_NEMA_GFX_H
#define LV_DRAW_NEMA_GFX_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_NEMA_GFX

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_draw_unit_t base_unit;
    lv_draw_task_t * task_act;
#if LV_USE_OS
    lv_thread_sync_t sync;
    lv_thread_t thread;
    volatile bool inited;
    volatile bool exit_status;
#endif
    uint32_t idx;
    nema_cmdlist_t cl;
#if LV_USE_NEMA_VG
    NEMA_VG_PAINT_HANDLE paint;
    NEMA_VG_GRAD_HANDLE gradient;
    NEMA_VG_PATH_HANDLE path;
#endif
} lv_draw_nema_gfx_unit_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_draw_nema_gfx_init(void);

void lv_draw_nema_gfx_deinit(void);

void lv_draw_nema_gfx_fill(lv_draw_task_t * t,
                           const lv_draw_fill_dsc_t * dsc, const lv_area_t * coords);

void lv_draw_nema_gfx_triangle(lv_draw_task_t * t, const lv_draw_triangle_dsc_t * dsc);

void lv_draw_nema_gfx_img(lv_draw_task_t * t, const lv_draw_image_dsc_t * dsc,
                          const lv_area_t * coords);

void lv_draw_nema_gfx_label(lv_draw_task_t * t, const lv_draw_label_dsc_t * dsc,
                            const lv_area_t * coords);

void lv_draw_nema_gfx_label_init(lv_draw_unit_t * draw_unit);

void lv_draw_nema_gfx_layer(lv_draw_task_t * t, const lv_draw_image_dsc_t * draw_dsc,
                            const lv_area_t * coords);

void lv_draw_nema_gfx_line(lv_draw_task_t * t, const lv_draw_line_dsc_t * dsc);

void lv_draw_nema_gfx_border(lv_draw_task_t * t, const lv_draw_border_dsc_t * dsc,
                             const lv_area_t * coords);

void lv_draw_nema_gfx_arc(lv_draw_task_t * t, const lv_draw_arc_dsc_t * dsc,
                          const lv_area_t * coords);

#if LV_USE_VECTOR_GRAPHIC && LV_USE_NEMA_VG
void lv_draw_nema_gfx_vector(lv_draw_task_t * t, const lv_draw_vector_dsc_t * dsc,
                             const lv_area_t * coords);
#endif

/**********************
 *      MACROS
 **********************/

#endif  /*LV_USE_NEMA_GFX*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_NEMA_GFX_H*/

/** @} */
