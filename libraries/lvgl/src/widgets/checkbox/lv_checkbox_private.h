#pragma once

/************************************************************************************************
 * @file    lv_checkbox_private.h
 *
 * @brief   Lv Checkbox Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../core/lv_obj_private.h"
#include "lv_checkbox.h"

/* Intra-component Headers */

/**
 * @defgroup lv_checkbox_private
 * @brief    lv_checkbox_private Firmware
 * @{
 */

#ifndef LV_CHECKBOX_PRIVATE_H
#define LV_CHECKBOX_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_CHECKBOX != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      TYPEDEFS
 **********************/

struct _lv_checkbox_t {
    lv_obj_t obj;
    char * txt;
    uint32_t static_txt : 1;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_CHECKBOX != 0 */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CHECKBOX_PRIVATE_H*/

/** @} */
