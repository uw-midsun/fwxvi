#pragma once

/************************************************************************************************
 * @file    lv_canvas_private.h
 *
 * @brief   Lv Canvas Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../image/lv_image_private.h"
#include "lv_canvas.h"

/* Intra-component Headers */

/**
 * @defgroup lv_canvas_private
 * @brief    lv_canvas_private Firmware
 * @{
 */

#ifndef LV_CANVAS_PRIVATE_H
#define LV_CANVAS_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_CANVAS != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/** Canvas data */
struct _lv_canvas_t {
    lv_image_t img;
    lv_draw_buf_t * draw_buf;
    lv_draw_buf_t static_buf;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_CANVAS != 0 */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CANVAS_PRIVATE_H*/

/** @} */
