#pragma once

/************************************************************************************************
 * @file    lv_opengles_shader.h
 *
 * @brief   Lv Opengles Shader
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../opengl_shader/lv_opengl_shader_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_opengles_shader
 * @brief    lv_opengles_shader Firmware
 * @{
 */

#ifndef LV_OPENGLES_SHADER_H
#define LV_OPENGLES_SHADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_OPENGLES

/**********************
 * GLOBAL PROTOTYPES
 **********************/

char * lv_opengles_shader_get_vertex(lv_opengl_glsl_version_t version);
char * lv_opengles_shader_get_fragment(lv_opengl_glsl_version_t version);
void lv_opengles_shader_get_source(lv_opengl_shader_portions_t * portions, lv_opengl_glsl_version_t version);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_OPENGLES*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_OPENGLES_SHADER_H*/

/** @} */
