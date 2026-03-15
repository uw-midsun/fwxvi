#pragma once

/************************************************************************************************
 * @file    lv_draw_nanovg.h
 *
 * @brief   Lv Draw Nanovg
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../misc/lv_types.h"

/* Intra-component Headers */

/**
 * @defgroup lv_draw_nanovg
 * @brief    lv_draw_nanovg Firmware
 * @{
 */

#ifndef LV_DRAW_NANOVG_H
#define LV_DRAW_NANOVG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_DRAW_NANOVG

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
 * Initialize NanoVG rendering
 */
void lv_draw_nanovg_init(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_NANOVG*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_NANOVG_H*/

/** @} */
