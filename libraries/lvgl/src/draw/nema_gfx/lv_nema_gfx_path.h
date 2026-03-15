#pragma once

/************************************************************************************************
 * @file    lv_nema_gfx_path.h
 *
 * @brief   Lv Nema Gfx Path
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "lv_draw_nema_gfx.h"

/* Intra-component Headers */

/**
 * @defgroup lv_nema_gfx_path
 * @brief    lv_nema_gfx_path Firmware
 * @{
 */

#ifndef LV_NEMA_GFX_PATH_H
#define LV_NEMA_GFX_PATH_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_NEMA_GFX
#if LV_USE_NEMA_VG

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    NEMA_VG_PATH_HANDLE path;
    NEMA_VG_PAINT_HANDLE paint;
    float * data;
    uint8_t * seg;
    uint32_t data_size;
    uint32_t seg_size;
} lv_nema_gfx_path_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_nema_gfx_path_t * lv_nema_gfx_path_create(void);

void lv_nema_gfx_path_alloc(lv_nema_gfx_path_t * nema_gfx_path);

void lv_nema_gfx_path_destroy(lv_nema_gfx_path_t * nema_gfx_path);

void lv_nema_gfx_path_move_to(lv_nema_gfx_path_t * nema_gfx_path,
                              float x, float y);

void lv_nema_gfx_path_line_to(lv_nema_gfx_path_t * nema_gfx_path,
                              float x, float y);

void lv_nema_gfx_path_quad_to(lv_nema_gfx_path_t * nema_gfx_path,
                              float cx, float cy,
                              float x, float y);

void lv_nema_gfx_path_cubic_to(lv_nema_gfx_path_t * nema_gfx_path,
                               float cx1, float cy1,
                               float cx2, float cy2,
                               float x, float y);

void lv_nema_gfx_path_end(lv_nema_gfx_path_t * nema_gfx_path);

#endif  /*LV_USE_NEMA_VG*/
#endif  /*LV_USE_NEMA_GFX*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_NEMA_GFX_PATH_H*/

/** @} */
