#pragma once

/************************************************************************************************
 * @file    lv_svg_decoder.h
 *
 * @brief   Lv Svg Decoder
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../draw/lv_image_decoder.h"
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_svg_decoder
 * @brief    lv_svg_decoder Firmware
 * @{
 */

#ifndef LV_SVG_DECODER_H
#define LV_SVG_DECODER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_SVG

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
 * Register the SVG decoder functions in LVGL
 */
void lv_svg_decoder_init(void);

void lv_svg_decoder_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_SVG*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SVG_DECODER_H*/

/** @} */
