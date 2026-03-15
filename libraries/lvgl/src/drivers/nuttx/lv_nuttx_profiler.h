#pragma once

/************************************************************************************************
 * @file    lv_nuttx_profiler.h
 *
 * @brief   Lv Nuttx Profiler
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_nuttx_profiler
 * @brief    lv_nuttx_profiler Firmware
 * @{
 */

#ifndef LV_NUTTX_PROFILER_H
#define LV_NUTTX_PROFILER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_NUTTX && LV_USE_PROFILER && LV_USE_PROFILER_BUILTIN

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_nuttx_profiler_init(void);
void lv_nuttx_profiler_set_file(const char * file);
void lv_nuttx_profiler_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_NUTTX && LV_USE_PROFILER && LV_USE_PROFILER_BUILTIN*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_NUTTX_PROFILER_H*/

/** @} */
