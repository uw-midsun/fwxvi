#pragma once

/************************************************************************************************
 * @file    lv_renesas_glcdc.h
 *
 * @brief   Lv Renesas Glcdc
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../../display/lv_display.h"

/* Intra-component Headers */

/**
 * @defgroup lv_renesas_glcdc
 * @brief    lv_renesas_glcdc Firmware
 * @{
 */

#ifndef LV_RENESAS_GLCDC_H
#define LV_RENESAS_GLCDC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_RENESAS_GLCDC

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a display using Renesas' GLCDC peripheral in DIRECT render mode
 * @return  pointer to the created display
 */
lv_display_t * lv_renesas_glcdc_direct_create(void);

/**
 * Create a display using Renesas' GLCDC peripheral in PARTIAL render mode
 * @param buf1      first buffer
 * @param buf2      second buffer (can be `NULL`)
 * @param buf_size  buffer size in byte
 * @return          pointer to the created display
 */
lv_display_t * lv_renesas_glcdc_partial_create(void * buf1, void * buf2, size_t buf_size);

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_RENESAS_GLCDC */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_RENESAS_GLCDC_H */

/** @} */
