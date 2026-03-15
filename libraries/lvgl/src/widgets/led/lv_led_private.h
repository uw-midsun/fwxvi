#pragma once

/************************************************************************************************
 * @file    lv_led_private.h
 *
 * @brief   Lv Led Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../core/lv_obj_private.h"
#include "lv_led.h"

/* Intra-component Headers */

/**
 * @defgroup lv_led_private
 * @brief    lv_led_private Firmware
 * @{
 */

#ifndef LV_LED_PRIVATE_H
#define LV_LED_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_LED

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/** Data of led */
struct _lv_led_t {
    lv_obj_t obj;
    lv_color_t color;
    uint8_t bright;     /**< Current brightness of the LED (0..255)*/
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_LED */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_LED_PRIVATE_H*/

/** @} */
