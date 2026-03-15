#pragma once

/************************************************************************************************
 * @file    lv_gltf_view_shader.h
 *
 * @brief   Lv Gltf View Shader
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../../../drivers/opengles/opengl_shader/lv_opengl_shader_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_gltf_view_shader
 * @brief    lv_gltf_view_shader Firmware
 * @{
 */

#ifndef LV_GLTF_VIEW_SHADER_H
#define LV_GLTF_VIEW_SHADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_GLTF

/**********************
 * GLOBAL PROTOTYPES
 **********************/

char *lv_gltf_view_shader_get_vertex(void);
char *lv_gltf_view_shader_get_fragment(void);
void lv_gltf_view_shader_get_src(lv_opengl_shader_portions_t *shaders);
void lv_gltf_view_shader_get_env(lv_opengl_shader_portions_t *shaders);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_GLTF*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_GLTF_VIEW_SHADER_H*/

/** @} */
