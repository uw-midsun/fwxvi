#pragma once

/************************************************************************************************
 * @file    lv_tileview_private.h
 *
 * @brief   Lv Tileview Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../core/lv_obj_private.h"
#include "lv_tileview.h"

/* Intra-component Headers */

/**
 * @defgroup lv_tileview_private
 * @brief    lv_tileview_private Firmware
 * @{
 */

#ifndef LV_TILEVIEW_PRIVATE_H
#define LV_TILEVIEW_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_TILEVIEW

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      TYPEDEFS
 **********************/
struct _lv_tileview_t {
    lv_obj_t obj;
    lv_obj_t * tile_act;
};

struct _lv_tileview_tile_t {
    lv_obj_t obj;
    lv_dir_t dir;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_TILEVIEW */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TILEVIEW_PRIVATE_H*/

/** @} */
