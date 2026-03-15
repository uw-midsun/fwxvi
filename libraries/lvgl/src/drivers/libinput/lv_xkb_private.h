#pragma once

/************************************************************************************************
 * @file    lv_xkb_private.h
 *
 * @brief   Lv Xkb Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "lv_xkb.h"

/* Intra-component Headers */

/**
 * @defgroup lv_xkb_private
 * @brief    lv_xkb_private Firmware
 * @{
 */

#ifndef LV_XKB_PRIVATE_H
#define LV_XKB_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if defined(LV_LIBINPUT_XKB) && LV_LIBINPUT_XKB

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      TYPEDEFS
 **********************/

struct _lv_xkb_t {
    struct xkb_keymap * keymap;
    struct xkb_state * state;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* defined(LV_LIBINPUT_XKB) && LV_LIBINPUT_XKB */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_XKB_PRIVATE_H*/

/** @} */
