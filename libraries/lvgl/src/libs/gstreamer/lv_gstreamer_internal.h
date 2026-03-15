#pragma once

/************************************************************************************************
 * @file    lv_gstreamer_internal.h
 *
 * @brief   Lv Gstreamer Internal
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <gst/gst.h>
#include <gst/video/video.h>

/* Inter-component Headers */
#include "../../lv_conf_internal.h"
#include "../../widgets/image/lv_image_private.h"
#include "lv_gstreamer.h"

/* Intra-component Headers */

/**
 * @defgroup lv_gstreamer_internal
 * @brief    lv_gstreamer_internal Firmware
 * @{
 */

#ifndef LV_GSTREAMER_INTERNAL_H
#define LV_GSTREAMER_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_GSTREAMER

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

struct _lv_gstreamer_t {
    lv_image_t image;
    lv_image_dsc_t frame;
    GstVideoInfo video_info;
    GstMapInfo last_map_info;
    GstBuffer * last_buffer;
    GstSample * last_sample;
    GstElement * pipeline;
    GstElement * audio_convert;
    GstElement * video_convert;
    GstElement * audio_volume;
    lv_timer_t * gstreamer_timer;
    GAsyncQueue * frame_queue;
    bool is_video_info_valid;
};

typedef struct {
    uint8_t * frame_data;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    size_t data_size;
} frame_data_t;

typedef struct _lv_gstreamer_t lv_gstreamer_t;

LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_gstreamer_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_GSTREAMER != 0 */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_GSTREAMER_INTERNAL_H*/

/** @} */
