#pragma once

/************************************************************************************************
 * @file    lv_init.h
 *
 * @brief   Lv Init
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "lv_conf_internal.h"
#include "misc/lv_types.h"

/* Intra-component Headers */

/**
 * @defgroup lv_init
 * @brief    lv_init Firmware
 * @{
 */

#ifndef LV_INIT_H
#define LV_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

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
 * Initialize LVGL library.
 * Should be called before any other LVGL related function.
 */
void lv_init(void);

/**
 * Deinit the 'lv' library
 */
void lv_deinit(void);

/**
 * Returns whether the 'lv' library is currently initialized
 */
bool lv_is_initialized(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_INIT_H*/

/** @} */
