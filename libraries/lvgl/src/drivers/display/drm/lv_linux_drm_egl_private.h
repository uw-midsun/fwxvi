#pragma once

/************************************************************************************************
 * @file    lv_linux_drm_egl_private.h
 *
 * @brief   Lv Linux Drm Egl Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <xf86drmMode.h>

/* Inter-component Headers */
#include "../../../lv_conf_internal.h"
#include "../../opengles/lv_opengles_egl.h"
#include "../../opengles/lv_opengles_egl_private.h"
#include "../../opengles/lv_opengles_texture_private.h"
#include "lv_linux_drm.h"

/* Intra-component Headers */

/**
 * @defgroup lv_linux_drm_egl_private
 * @brief    lv_linux_drm_egl_private Firmware
 * @{
 */

#ifndef LV_LINUX_DRM_EGL_PRIVATE_H
#define LV_LINUX_DRM_EGL_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_LINUX_DRM && LV_LINUX_DRM_USE_EGL

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_opengles_texture_t texture;
    lv_display_t * display;
    lv_opengles_egl_t * egl_ctx;
    lv_egl_interface_t egl_interface;

    drmModeRes * drm_resources;
    drmModeConnector * drm_connector;
    drmModeEncoder * drm_encoder;
    drmModeCrtc * drm_crtc;
    drmModeModeInfo * drm_mode;

    struct gbm_device * gbm_dev;
    struct gbm_surface * gbm_surface;
    struct gbm_bo * gbm_bo_pending;
    struct gbm_bo * gbm_bo_flipped;
    struct gbm_bo * gbm_bo_presented;

    lv_linux_drm_select_mode_cb_t mode_select_cb;
    int fd;
    bool crtc_isset;
} lv_drm_ctx_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_LINUX_DRM && LV_LINUX_DRM_USE_EGL*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_LINUX_DRM_EGL_PRIVATE_H*/

/** @} */
