#pragma once

/************************************************************************************************
 * @file    lv_keyboard_private.h
 *
 * @brief   Lv Keyboard Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../buttonmatrix/lv_buttonmatrix_private.h"
#include "lv_keyboard.h"

/* Intra-component Headers */

/**
 * @defgroup lv_keyboard_private
 * @brief    lv_keyboard_private Firmware
 * @{
 */

#ifndef LV_KEYBOARD_PRIVATE_H
#define LV_KEYBOARD_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_KEYBOARD

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/** Data of keyboard */
struct _lv_keyboard_t {
    lv_buttonmatrix_t btnm;
    lv_obj_t * ta;              /**< Pointer to the assigned text area */
    lv_keyboard_mode_t mode;    /**< Key map type */
    uint8_t popovers : 1;       /**< Show button titles in popovers on press */
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_KEYBOARD */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_KEYBOARD_PRIVATE_H*/

/** @} */
