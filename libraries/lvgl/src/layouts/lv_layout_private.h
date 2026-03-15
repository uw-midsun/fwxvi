#pragma once

/************************************************************************************************
 * @file    lv_layout_private.h
 *
 * @brief   Lv Layout Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "lv_layout.h"

/* Intra-component Headers */

/**
 * @defgroup lv_layout_private
 * @brief    lv_layout_private Firmware
 * @{
 */

#ifndef LV_LAYOUT_PRIVATE_H
#define LV_LAYOUT_PRIVATE_H

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

typedef struct {
    lv_layout_callbacks_t callbacks;
    void * user_data;
} lv_layout_dsc_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_layout_init(void);

void lv_layout_deinit(void);

bool lv_layout_get_min_size(lv_obj_t * obj, int32_t * size, bool width);

/**
 * Update the layout of a widget
 * @param obj   pointer to a widget
 */
void lv_layout_apply(lv_obj_t * obj);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_LAYOUT_PRIVATE_H*/

/** @} */
