#pragma once

/************************************************************************************************
 * @file    lv_test_helpers.h
 *
 * @brief   Lv Test Helpers
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"
#include "../../misc/lv_types.h"
#include "../../stdlib/lv_mem.h"

/* Intra-component Headers */

/**
 * @defgroup lv_test_helpers
 * @brief    lv_test_helpers Firmware
 * @{
 */

#ifndef LV_TEST_HELPERS_H
#define LV_TEST_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_TEST

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Emulate a delay. It's not real delay, but it tricks LVGL to think that the
 * required time has been elapsed.
 * `lv_timer_handler` is called after each millisecond, meaning all the events
 * will be fired inside this function.
 * At the end the animations and display will be also updated.
 * @param ms    the number of milliseconds to pass
 */
void lv_test_wait(uint32_t ms);

/**
 * Emulates some time passing.
 * Update the animations and the display only once at the end.
 * @param ms    the number of milliseconds to pass
 */
void lv_test_fast_forward(uint32_t ms);

#if LV_USE_STDLIB_MALLOC != LV_STDLIB_BUILTIN
/* Skip checking heap as we don't have the info available */
#define LV_HEAP_CHECK(x) do {} while(0)
/* Pick a non-zero value */
#define lv_test_get_free_mem() (65536)
#else
#define LV_HEAP_CHECK(x) x

static inline size_t lv_test_get_free_mem(void)
{
    lv_mem_monitor_t m1;
    lv_mem_monitor(&m1);
    return m1.free_size;
}
#endif /* LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN */

/**********************
 *      MACROS
 **********************/

#define LV_TEST_WIDTH_TO_STRIDE(w, px_size) ((((w) * (px_size) + (LV_DRAW_BUF_STRIDE_ALIGN - 1)) / LV_DRAW_BUF_STRIDE_ALIGN) * LV_DRAW_BUF_STRIDE_ALIGN)

#endif /*LV_USE_TEST*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TEST_HELPERS_H*/

/** @} */
