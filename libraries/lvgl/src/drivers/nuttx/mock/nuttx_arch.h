#pragma once

/************************************************************************************************
 * @file    nuttx_arch.h
 *
 * @brief   Nuttx Arch
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup nuttx_arch
 * @brief    nuttx_arch Firmware
 * @{
 */

#ifndef NUTTX_ARCH_H
#define NUTTX_ARCH_H

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

/**********************
 * GLOBAL PROTOTYPES
 **********************/

static inline unsigned long up_perf_getfreq(void)
{
    return 1000000; /*1 MHz for mock*/
}

static inline uint32_t up_perf_gettime(void)
{
    return 0;
}

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*NUTTX_ARCH_H*/

/** @} */
