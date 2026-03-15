#pragma once

/************************************************************************************************
 * @file    lv_rtthread.h
 *
 * @brief   Lv Rtthread
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <rtthread.h>
#include <stdbool.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup lv_rtthread
 * @brief    lv_rtthread Firmware
 * @{
 */

#ifndef LV_RTTHREAD_H
#define LV_RTTHREAD_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_OS == LV_OS_RTTHREAD

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    rt_thread_t thread;
} lv_thread_t;

typedef struct {
    rt_mutex_t mutex;
} lv_mutex_t;

typedef struct {
    rt_sem_t sem;
} lv_thread_sync_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_OS == LV_OS_RTTHREAD*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_RTTHREAD_H*/

/** @} */
