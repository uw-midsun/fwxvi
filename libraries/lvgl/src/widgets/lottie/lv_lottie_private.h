#pragma once

/************************************************************************************************
 * @file    lv_lottie_private.h
 *
 * @brief   Lv Lottie Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <thorvg_capi.h>

/* Inter-component Headers */
#include "../../libs/thorvg/thorvg_capi.h"
#include "../../lv_conf_internal.h"
#include "../canvas/lv_canvas_private.h"
#include "lv_lottie.h"

/* Intra-component Headers */

/**
 * @defgroup lv_lottie_private
 * @brief    lv_lottie_private Firmware
 * @{
 */

#ifndef LV_LOTTIE_PRIVATE_H
#define LV_LOTTIE_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_LOTTIE

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

#if LV_USE_THORVG_EXTERNAL
#else
#endif

typedef struct {
    lv_canvas_t canvas;
    Tvg_Paint * tvg_paint;
    Tvg_Canvas * tvg_canvas;
    Tvg_Animation * tvg_anim;
    lv_anim_t * anim;
    int32_t last_rendered_time;
} lv_lottie_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /*LV_LOTTIE_H*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_LOTTIE_PRIVATE_H*/

/** @} */
