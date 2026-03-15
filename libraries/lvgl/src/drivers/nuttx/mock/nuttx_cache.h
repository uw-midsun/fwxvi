#pragma once

/************************************************************************************************
 * @file    nuttx_cache.h
 *
 * @brief   Nuttx Cache
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup nuttx_cache
 * @brief    nuttx_cache Firmware
 * @{
 */

#ifndef NUTTX_CACHE_H
#define NUTTX_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/*********************
 *      TYPEDEFS
 *********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

static inline void up_invalidate_dcache(uintptr_t start, uintptr_t end)
{
    (void)start;
    (void)end;
}

static inline void up_flush_dcache(uintptr_t start, uintptr_t end)
{
    (void)start;
    (void)end;
}

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*NUTTX_CACHE_H*/

/** @} */
