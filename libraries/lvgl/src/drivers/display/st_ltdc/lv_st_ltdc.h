#pragma once

/************************************************************************************************
 * @file    lv_st_ltdc.h
 *
 * @brief   Lv St Ltdc
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../../display/lv_display.h"
#include "../../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_st_ltdc
 * @brief    lv_st_ltdc Firmware
 * @{
 */

#ifndef LV_ST_LTDC_H
#define LV_ST_LTDC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_ST_LTDC

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a direct render mode display bound to a LTDC layer.
 * @param fb_adr_1      The LTDC layer's framebuffer memory address.
 * @param fb_adr_2      An additional framebuffer-sized buffer to use for double buffering, or `NULL`.
 * @param layer_idx     The LTDC layer number to bind the display to. Typically 0 or 1.
 * @return              The display.
 */
lv_display_t * lv_st_ltdc_create_direct(void * fb_adr_1, void * fb_adr_2, uint32_t layer_idx);

/**
 * Create a partial render mode display bound to a LTDC layer. The layer's framebuffer is flushed to internally.
 * Enable `LV_ST_LTDC_USE_DMA2D_FLUSH` for parallel flushing.
 * @param render_buf_1  A render buffer.
 * @param render_buf_2  An additional render buffer for double-buffering, or `NULL`.
 * @param buf_size      The size of the buffer(s) in bytes.
 * @param layer_idx     The LTDC layer number to bind the display to. Typically 0 or 1.
 * @return              The display.
 */
lv_display_t * lv_st_ltdc_create_partial(void * render_buf_1, void * render_buf_2, uint32_t buf_size,
                                         uint32_t layer_idx);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_ST_LTDC*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_ST_LTDC_H*/

/** @} */
