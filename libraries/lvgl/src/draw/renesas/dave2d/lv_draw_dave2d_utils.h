#pragma once

/************************************************************************************************
 * @file    lv_draw_dave2d_utils.h
 *
 * @brief   Lv Draw Dave2D Utils
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup lv_draw_dave2d_utils
 * @brief    lv_draw_dave2d_utils Firmware
 * @{
 */

#ifndef LV_DRAW_DAVE2D_UTILS_H
#define LV_DRAW_DAVE2D_UTILS_H

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

d2_color lv_draw_dave2d_lv_colour_to_d2_colour(lv_color_t color);

d2_u32 lv_draw_dave2d_lv_colour_fmt_to_d2_fmt(lv_color_format_t colour_format);

void d2_framebuffer_from_layer(d2_device * handle, lv_layer_t * layer);

bool lv_draw_dave2d_is_dest_cf_supported(lv_color_format_t cf);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_DAVE2D_UTILS_H*/

/** @} */
