#pragma once

/************************************************************************************************
 * @file    lv_rlottie.h
 *
 * @brief   Lv Rlottie
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_rlottie
 * @brief    lv_rlottie Firmware
 * @{
 */

#ifndef LV_RLOTTIE_H
#define LV_RLOTTIE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_RLOTTIE

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    LV_RLOTTIE_CTRL_FORWARD  = 0,
    LV_RLOTTIE_CTRL_BACKWARD = 1,
    LV_RLOTTIE_CTRL_PAUSE    = 2,
    LV_RLOTTIE_CTRL_PLAY     = 0, /* Yes, play = 0 is the default mode */
    LV_RLOTTIE_CTRL_LOOP     = 8,
} lv_rlottie_ctrl_t;

LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_rlottie_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_obj_t * lv_rlottie_create_from_file(lv_obj_t * parent, int32_t width, int32_t height, const char * path);

lv_obj_t * lv_rlottie_create_from_raw(lv_obj_t * parent, int32_t width, int32_t height,
                                      const char * rlottie_desc);

void lv_rlottie_set_play_mode(lv_obj_t * rlottie, const lv_rlottie_ctrl_t ctrl);
void lv_rlottie_set_current_frame(lv_obj_t * rlottie, const size_t goto_frame);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_RLOTTIE*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_RLOTTIE_H*/

/** @} */
