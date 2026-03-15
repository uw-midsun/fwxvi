#pragma once

/************************************************************************************************
 * @file    lv_draw_eve.h
 *
 * @brief   Lv Draw Eve
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"
#include "lv_draw_eve_target.h"

/* Intra-component Headers */

/**
 * @defgroup lv_draw_eve
 * @brief    lv_draw_eve Firmware
 * @{
 */

/*  Created on: 3 dic 2023
 *      Author: juanj
 *
 *  Modified by LVGL
 */

#ifndef LV_DRAW_EVE_H
#define LV_DRAW_EVE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_DRAW_EVE

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_draw_eve_init(void);

void lv_draw_eve_set_display_data(lv_display_t * disp, const lv_draw_eve_parameters_t * params,
                                  lv_draw_eve_operation_cb_t op_cb);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DRAW_EVE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* LV_DRAW_EVE_H */

/** @} */
