#pragma once

/************************************************************************************************
 * @file    lv_evdev_private.h
 *
 * @brief   Lv Evdev Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "lv_evdev.h"

/* Intra-component Headers */

/**
 * @defgroup lv_evdev_private
 * @brief    lv_evdev_private Firmware
 * @{
 */

#ifndef LV_EVDEV_PRIVATE_H
#define LV_EVDEV_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_EVDEV

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_evdev_deinit(void);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_EVDEV*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_EVDEV_PRIVATE_H*/

/** @} */
