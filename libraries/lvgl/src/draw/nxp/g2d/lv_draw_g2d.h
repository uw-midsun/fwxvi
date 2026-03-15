#pragma once

/************************************************************************************************
 * @file    lv_draw_g2d.h
 *
 * @brief   Lv Draw G2D
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../../lv_conf_internal.h"
#include "../../sw/lv_draw_sw_private.h"

/* Intra-component Headers */

/**
 * @defgroup lv_draw_g2d
 * @brief    lv_draw_g2d Firmware
 * @{
 */

/**
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LV_DRAW_G2D_H
#define LV_DRAW_G2D_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_G2D
#if LV_USE_DRAW_G2D || LV_USE_ROTATE_G2D

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct lv_draw_g2d_unit {
    lv_draw_unit_t base_unit;
#if LV_USE_OS
    lv_draw_sw_thread_dsc_t thread_dsc;
#else
    lv_draw_task_t * task_act;
#endif
} lv_draw_g2d_unit_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_draw_g2d_init(void);

void lv_draw_g2d_deinit(void);

void lv_draw_buf_g2d_init_handlers(void);

void lv_draw_g2d_fill(lv_draw_task_t * t);

void lv_draw_g2d_img(lv_draw_task_t * t);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_G2D || LV_USE_ROTATE_G2D*/
#endif /*LV_USE_G2D*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_G2D_H*/

/** @} */
