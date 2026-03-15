#pragma once

/************************************************************************************************
 * @file    lv_opengles_debug.h
 *
 * @brief   Lv Opengles Debug
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_opengles_debug
 * @brief    lv_opengles_debug Firmware
 * @{
 */

#ifndef LV_OPENGLES_DEBUG_H
#define LV_OPENGLES_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_OPENGLES

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

#if LV_USE_OPENGLES_DEBUG

void GLClearError(void);

void GLLogCall(const char * function, const char * file, int line);

#endif

/**********************
 *      MACROS
 **********************/

#if LV_USE_OPENGLES_DEBUG
#define GL_CALL(x) do {\
        GLClearError();\
        x;\
        GLLogCall(#x, __FILE__, __LINE__);\
    } while(0)
#else
#define GL_CALL(x) x
#endif

#endif /* LV_USE_OPENGLES */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_OPENGLES_DEBUG_H */

/** @} */
