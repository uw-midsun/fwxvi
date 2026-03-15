#pragma once

/************************************************************************************************
 * @file    lv_tick_private.h
 *
 * @brief   Lv Tick Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "lv_tick.h"

/* Intra-component Headers */

/**
 * @defgroup lv_tick_private
 * @brief    lv_tick_private Firmware
 * @{
 */

#ifndef LV_TICK_PRIVATE_H
#define LV_TICK_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    volatile uint32_t sys_time;
    volatile uint8_t sys_irq_flag;
    lv_tick_get_cb_t tick_get_cb;
    lv_delay_cb_t delay_cb;
} lv_tick_state_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TICK_PRIVATE_H*/

/** @} */
