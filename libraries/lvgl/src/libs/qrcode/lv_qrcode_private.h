#pragma once

/************************************************************************************************
 * @file    lv_qrcode_private.h
 *
 * @brief   Lv Qrcode Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../widgets/canvas/lv_canvas_private.h"
#include "lv_qrcode.h"

/* Intra-component Headers */

/**
 * @defgroup lv_qrcode_private
 * @brief    lv_qrcode_private Firmware
 * @{
 */

#ifndef LV_QRCODE_PRIVATE_H
#define LV_QRCODE_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_QRCODE

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*Data of qrcode*/
struct _lv_qrcode_t {
    lv_canvas_t canvas;
    lv_color_t dark_color;
    lv_color_t light_color;
    int32_t quiet_zone;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_QRCODE */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_QRCODE_PRIVATE_H*/

/** @} */
