#pragma once

/************************************************************************************************
 * @file    lv_tabview_private.h
 *
 * @brief   Lv Tabview Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../core/lv_obj_private.h"
#include "lv_tabview.h"

/* Intra-component Headers */

/**
 * @defgroup lv_tabview_private
 * @brief    lv_tabview_private Firmware
 * @{
 */

#ifndef LV_TABVIEW_PRIVATE_H
#define LV_TABVIEW_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_TABVIEW

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      TYPEDEFS
 **********************/

struct _lv_tabview_t {
    lv_obj_t obj;
    uint32_t tab_cur;
    lv_dir_t tab_pos;
    int32_t tab_bar_size;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_TABVIEW */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TABVIEW_PRIVATE_H*/

/** @} */
