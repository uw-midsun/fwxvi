#pragma once

/************************************************************************************************
 * @file    lv_draw_sdl.h
 *
 * @brief   Lv Draw Sdl
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../display/lv_display.h"
#include "../../draw/lv_draw_arc.h"
#include "../../draw/lv_draw_image.h"
#include "../../draw/lv_draw_label.h"
#include "../../draw/lv_draw_line.h"
#include "../../draw/lv_draw_rect.h"
#include "../../draw/lv_draw_triangle.h"
#include "../../misc/lv_area.h"
#include "../../misc/lv_color.h"
#include "../../osal/lv_os_private.h"
#include "../lv_draw.h"

/* Intra-component Headers */

/**
 * @defgroup lv_draw_sdl
 * @brief    lv_draw_sdl Firmware
 * @{
 */

#ifndef LV_DRAW_SDL_H
#define LV_DRAW_SDL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_DRAW_SDL

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_draw_sdl_init(void);

void /* LV_ATTRIBUTE_FAST_MEM */ lv_draw_sdl_image(lv_draw_unit_t * draw_unit, const lv_draw_image_dsc_t * draw_dsc,
                                                   const lv_area_t * coords);

void lv_draw_sdl_fill(lv_draw_unit_t * draw_unit, const lv_draw_fill_dsc_t * dsc, const lv_area_t * coords);

void lv_draw_sdl_border(lv_draw_unit_t * draw_unit, const lv_draw_border_dsc_t * dsc, const lv_area_t * coords);

void lv_draw_sdl_box_shadow(lv_draw_unit_t * draw_unit, const lv_draw_box_shadow_dsc_t * dsc, const lv_area_t * coords);

void lv_draw_sdl_label(lv_draw_unit_t * draw_unit, const lv_draw_label_dsc_t * dsc, const lv_area_t * coords);

void lv_draw_sdl_arc(lv_draw_unit_t * draw_unit, const lv_draw_arc_dsc_t * dsc, const lv_area_t * coords);

void /* LV_ATTRIBUTE_FAST_MEM */ lv_draw_sdl_line(lv_draw_unit_t * draw_unit, const lv_draw_line_dsc_t * dsc);

void lv_draw_sdl_layer(lv_draw_unit_t * draw_unit, const lv_draw_image_dsc_t * draw_dsc, const lv_area_t * coords);

void lv_draw_sdl_triangle(lv_draw_unit_t * draw_unit, const lv_draw_triangle_dsc_t * dsc);

void lv_draw_sdl_mask_rect(lv_draw_unit_t * draw_unit, const lv_draw_mask_rect_dsc_t * dsc, const lv_area_t * coords);

/***********************
 * GLOBAL VARIABLES
 ***********************/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_SDL*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_SDL_H*/

/** @} */
