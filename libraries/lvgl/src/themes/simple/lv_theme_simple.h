#pragma once

/************************************************************************************************
 * @file    lv_theme_simple.h
 *
 * @brief   Lv Theme Simple
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../display/lv_display.h"
#include "../lv_theme.h"

/* Intra-component Headers */

/**
 * @defgroup lv_theme_simple
 * @brief    lv_theme_simple Firmware
 * @{
 */

#ifndef LV_THEME_SIMPLE_H
#define LV_THEME_SIMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_THEME_SIMPLE

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
 * Initialize the theme
 * @param disp pointer to display
 * @return a pointer to reference this theme later
 */
lv_theme_t * lv_theme_simple_init(lv_display_t * disp);

/**
* Check if the theme is initialized
* @return true if default theme is initialized, false otherwise
*/
bool lv_theme_simple_is_inited(void);

/**
 * Get simple theme
 * @return a pointer to simple theme, or NULL if this is not initialized
 */
lv_theme_t * lv_theme_simple_get(void);

/**
 * Deinitialize the simple theme
 */
void lv_theme_simple_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_THEME_SIMPLE_H*/

/** @} */
