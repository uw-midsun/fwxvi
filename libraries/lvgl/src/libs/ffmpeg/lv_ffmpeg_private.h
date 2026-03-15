#pragma once

/************************************************************************************************
 * @file    lv_ffmpeg_private.h
 *
 * @brief   Lv Ffmpeg Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../widgets/image/lv_image_private.h"
#include "lv_ffmpeg.h"

/* Intra-component Headers */

/**
 * @defgroup lv_ffmpeg_private
 * @brief    lv_ffmpeg_private Firmware
 * @{
 */

#ifndef LV_FFMPEG_PRIVATE_H
#define LV_FFMPEG_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_FFMPEG != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

struct _lv_ffmpeg_player_t {
    lv_image_t img;
    lv_timer_t * timer;
    lv_image_dsc_t imgdsc;
    bool auto_restart;
    struct ffmpeg_context_s * ffmpeg_ctx;
    const char * decoder_name;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_FFMPEG*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_FFMPEG_PRIVATE_H*/

/** @} */
