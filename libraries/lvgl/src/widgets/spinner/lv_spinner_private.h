#pragma once

/************************************************************************************************
 * @file    lv_spinner_private.h
 *
 * @brief   Lv Spinner Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../arc/lv_arc_private.h"
#include "lv_spinner.h"

/* Intra-component Headers */

/**
 * @defgroup lv_spinner_private
 * @brief    lv_spinner_private Firmware
 * @{
 */

#ifndef LV_SPINNER_PRIVATE_H
#define LV_SPINNER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_SPINNER

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

struct _lv_spinner_t {
    lv_arc_t arc;                 /**< Add the ancestor's type first */
    uint32_t duration;            /**< Anim duration in ms */
    uint32_t angle;               /**< Anim angle in degrees */
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /*LV_SPINNER_PRIVATE_H*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SPINNER_PRIVATE_H*/

/** @} */
