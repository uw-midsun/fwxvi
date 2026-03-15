#pragma once

/************************************************************************************************
 * @file    lv_msgbox_private.h
 *
 * @brief   Lv Msgbox Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../core/lv_obj_private.h"
#include "lv_msgbox.h"

/* Intra-component Headers */

/**
 * @defgroup lv_msgbox_private
 * @brief    lv_msgbox_private Firmware
 * @{
 */

#ifndef LV_MSGBOX_PRIVATE_H
#define LV_MSGBOX_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_MSGBOX

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      TYPEDEFS
 **********************/

struct _lv_msgbox_t {
    lv_obj_t obj;
    lv_obj_t * header;
    lv_obj_t * content;
    lv_obj_t * footer;
    lv_obj_t * title;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_MSGBOX */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_MSGBOX_PRIVATE_H*/

/** @} */
