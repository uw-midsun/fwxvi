#pragma once

/************************************************************************************************
 * @file    lv_win_private.h
 *
 * @brief   Lv Win Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../core/lv_obj_private.h"
#include "lv_win.h"

/* Intra-component Headers */

/**
 * @defgroup lv_win_private
 * @brief    lv_win_private Firmware
 * @{
 */

#ifndef LV_WIN_PRIVATE_H
#define LV_WIN_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_WIN

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      TYPEDEFS
 **********************/
struct _lv_win_t {
    lv_obj_t obj;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_WIN */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_WIN_PRIVATE_H*/

/** @} */
