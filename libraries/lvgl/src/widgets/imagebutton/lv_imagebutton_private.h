#pragma once

/************************************************************************************************
 * @file    lv_imagebutton_private.h
 *
 * @brief   Lv Imagebutton Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../core/lv_obj_private.h"
#include "lv_imagebutton.h"

/* Intra-component Headers */

/**
 * @defgroup lv_imagebutton_private
 * @brief    lv_imagebutton_private Firmware
 * @{
 */

#ifndef LV_IMAGEBUTTON_PRIVATE_H
#define LV_IMAGEBUTTON_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_IMAGEBUTTON != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

struct _lv_imagebutton_src_info_t {
    const void * img_src;
    lv_image_header_t header;
};

/** Data of image button */
struct _lv_imagebutton_t {
    lv_obj_t obj;
    lv_imagebutton_src_info_t src_mid[LV_IMAGEBUTTON_STATE_NUM];   /**< Store center images to each state */
    lv_imagebutton_src_info_t src_left[LV_IMAGEBUTTON_STATE_NUM];  /**< Store left side images to each state */
    lv_imagebutton_src_info_t src_right[LV_IMAGEBUTTON_STATE_NUM]; /**< Store right side images to each state */
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_IMAGEBUTTON != 0 */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_IMAGEBUTTON_PRIVATE_H*/

/** @} */
