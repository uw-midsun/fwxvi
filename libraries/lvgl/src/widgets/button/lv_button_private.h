#pragma once

/************************************************************************************************
 * @file    lv_button_private.h
 *
 * @brief   Lv Button Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../core/lv_obj_private.h"
#include "lv_button.h"

/* Intra-component Headers */

/**
 * @defgroup lv_button_private
 * @brief    lv_button_private Firmware
 * @{
 */

#ifndef LV_BUTTON_PRIVATE_H
#define LV_BUTTON_PRIVATE_H

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

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      TYPEDEFS
 **********************/

struct _lv_button_t {
    lv_obj_t obj;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_BUTTON != 0 */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_BUTTON_PRIVATE_H*/

/** @} */
