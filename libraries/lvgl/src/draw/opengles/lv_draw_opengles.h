#pragma once

/************************************************************************************************
 * @file    lv_draw_opengles.h
 *
 * @brief   Lv Draw Opengles
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_draw_opengles
 * @brief    lv_draw_opengles Firmware
 * @{
 */

#ifndef LV_DRAW_OPENGLES_H
#define LV_DRAW_OPENGLES_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_DRAW_OPENGLES

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_draw_opengles_init(void);
void lv_draw_opengles_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_OPENGLES*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_OPENGLES_H*/

/** @} */
