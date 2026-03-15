#pragma once

/************************************************************************************************
 * @file    lv_switch_private.h
 *
 * @brief   Lv Switch Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../core/lv_obj_private.h"
#include "lv_switch.h"

/* Intra-component Headers */

/**
 * @defgroup lv_switch_private
 * @brief    lv_switch_private Firmware
 * @{
 */

#ifndef LV_SWITCH_PRIVATE_H
#define LV_SWITCH_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_SWITCH != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      TYPEDEFS
 **********************/

struct _lv_switch_t {
    lv_obj_t obj;
    int32_t anim_state;
    lv_switch_orientation_t orientation : 3; /**< Orientation of switch*/
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_SWITCH != 0 */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SWITCH_PRIVATE_H*/

/** @} */
