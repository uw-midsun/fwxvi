#pragma once

/************************************************************************************************
 * @file    lv_button.h
 *
 * @brief   Lv Button
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../core/lv_obj.h"
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_button
 * @brief    lv_button Firmware
 * @{
 */

#ifndef LV_BUTTON_H
#define LV_BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_BUTTON != 0

/*********************
 *      DEFINES
 *********************/

LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_button_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a button object
 * @param parent    pointer to an object, it will be the parent of the new button
 * @return          pointer to the created button
 */
lv_obj_t * lv_button_create(lv_obj_t * parent);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_BUTTON*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_BUTTON_H*/

/** @} */
