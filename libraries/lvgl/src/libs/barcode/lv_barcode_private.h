#pragma once

/************************************************************************************************
 * @file    lv_barcode_private.h
 *
 * @brief   Lv Barcode Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../widgets/canvas/lv_canvas_private.h"
#include "lv_barcode.h"

/* Intra-component Headers */

/**
 * @defgroup lv_barcode_private
 * @brief    lv_barcode_private Firmware
 * @{
 */

#ifndef LV_BARCODE_PRIVATE_H
#define LV_BARCODE_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_BARCODE

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*Data of barcode*/
struct _lv_barcode_t {
    lv_canvas_t canvas;
    lv_color_t dark_color;
    lv_color_t light_color;
    uint16_t scale;
    lv_dir_t direction;
    bool tiled;
    lv_barcode_encoding_t encoding;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_BARCODE */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_BARCODE_PRIVATE_H*/

/** @} */
