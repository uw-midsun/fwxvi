#pragma once

/************************************************************************************************
 * @file    lv_cmsis_rtos2.h
 *
 * @brief   Lv Cmsis Rtos2
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "cmsis_os2.h"

/* Intra-component Headers */

/**
 * @defgroup lv_cmsis_rtos2
 * @brief    lv_cmsis_rtos2 Firmware
 * @{
 */

/*
 * Copyright (C) 2023 Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LV_CMSIS_RTOS2_H
#define LV_CMSIS_RTOS2_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_OS == LV_OS_CMSIS_RTOS2

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef osThreadId_t lv_thread_t;

typedef osMutexId_t lv_mutex_t;

typedef osEventFlagsId_t lv_thread_sync_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_OS == LV_OS_CMSIS_RTOS2*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CMSIS_RTOS2_H*/

/** @} */
