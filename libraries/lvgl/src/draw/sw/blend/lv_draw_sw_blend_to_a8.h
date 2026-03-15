#pragma once

/************************************************************************************************
 * @file    lv_draw_sw_blend_to_a8.h
 *
 * @brief   Lv Draw Sw Blend To A8
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../lv_draw_sw.h"

/* Intra-component Headers */

/**
 * @defgroup lv_draw_sw_blend_to_a8
 * @brief    lv_draw_sw_blend_to_a8 Firmware
 * @{
 */

#ifndef LV_DRAW_SW_BLEND_TO_A8_H
#define LV_DRAW_SW_BLEND_TO_A8_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_DRAW_SW

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void /* LV_ATTRIBUTE_FAST_MEM */ lv_draw_sw_blend_color_to_a8(lv_draw_sw_blend_fill_dsc_t * dsc);

void /* LV_ATTRIBUTE_FAST_MEM */ lv_draw_sw_blend_image_to_a8(lv_draw_sw_blend_image_dsc_t * dsc);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_SW*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_SW_BLEND_TO_A8_H*/

/** @} */
