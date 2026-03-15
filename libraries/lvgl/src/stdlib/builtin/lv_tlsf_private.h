#pragma once

/************************************************************************************************
 * @file    lv_tlsf_private.h
 *
 * @brief   Lv Tlsf Private
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../osal/lv_os_private.h"
#include "lv_tlsf.h"

/* Intra-component Headers */

/**
 * @defgroup lv_tlsf_private
 * @brief    lv_tlsf_private Firmware
 * @{
 */

#ifndef LV_TLSF_PRIVATE_H
#define LV_TLSF_PRIVATE_H

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN

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
#if LV_USE_OS
    lv_mutex_t mutex;
#endif
    lv_tlsf_t tlsf;
    size_t cur_used;
    size_t max_used;
    lv_ll_t  pool_ll;
} lv_tlsf_state_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN*/

#endif /*LV_TLSF_PRIVATE_H*/

/** @} */
