#pragma once

/************************************************************************************************
 * @file    lv_mqx.h
 *
 * @brief   Lv Mqx
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "lv_os_private.h"
#include "mqx.h"
#include "mutex.h"

/* Intra-component Headers */

/**
 * @defgroup lv_mqx
 * @brief    lv_mqx Firmware
 * @{
 */

#ifndef LV_MQX_H
#define LV_MQX_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_OS == LV_OS_MQX

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef _task_id lv_thread_t;

typedef MUTEX_STRUCT lv_mutex_t;

typedef LWSEM_STRUCT lv_thread_sync_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_OS == LV_OS_MQX*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_MQX_H*/

/** @} */
