#pragma once

/************************************************************************************************
 * @file    lv_opengles_egl.h
 *
 * @brief   Lv Opengles Egl
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"
#include "../../misc/lv_color.h"
#include "../../misc/lv_types.h"

/* Intra-component Headers */

/**
 * @defgroup lv_opengles_egl
 * @brief    lv_opengles_egl Firmware
 * @{
 */

#ifndef LV_OPENGLES_EGL_H
#define LV_OPENGLES_EGL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_EGL

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _lv_opengles_egl lv_opengles_egl_t;
typedef struct _lv_egl_interface lv_egl_interface_t;
typedef struct _lv_egl_config lv_egl_config_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_opengles_egl_t * lv_opengles_egl_context_create(const lv_egl_interface_t * interface);
lv_color_format_t lv_opengles_egl_color_format_from_egl_config(const lv_egl_config_t * config);

void lv_opengles_egl_update(lv_opengles_egl_t * ctx);
void lv_opengles_egl_clear(lv_opengles_egl_t * ctx);
void lv_opengles_egl_context_destroy(lv_opengles_egl_t * ctx);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_EGL*/
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_OPENGLES_EGL_H*/

/** @} */
