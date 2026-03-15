#pragma once

/************************************************************************************************
 * @file    nuttx_clock.h
 *
 * @brief   Nuttx Clock
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup nuttx_clock
 * @brief    nuttx_clock Firmware
 * @{
 */

#ifndef NUTTX_CLOCK_H
#define NUTTX_CLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

#define CONFIG_SCHED_CPULOAD 1

#define pthread_get_stacksize_np(tid) 0

/**********************
 *      TYPEDEFS
 **********************/

struct cpuload_s {
    volatile uint32_t total;   /* Total number of clock ticks */
    volatile uint32_t active;  /* Number of ticks while this thread was active */
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

static inline int clock_cpuload(int pid, struct cpuload_s * cpuload)
{
    (void)pid;
    (void)cpuload;
    return -1;
}

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*NUTTX_CLOCK_H*/

/** @} */
