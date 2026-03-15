#pragma once

/************************************************************************************************
 * @file    lv_test_fs.h
 *
 * @brief   Lv Test Fs
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"
#include "../../misc/lv_types.h"

/* Intra-component Headers */

/**
 * @defgroup lv_test_fs
 * @brief    lv_test_fs Firmware
 * @{
 */

#ifndef LV_TEST_FS_H
#define LV_TEST_FS_H

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
 * Initialize the test file system driver
 */
void lv_test_fs_init(void);

/**
 * Set whether the test file system is ready
 * @param ready true: ready, false: not ready
 */
void lv_test_fs_set_ready(bool ready);

/**
 * Set whether the open callback of the test file system is cleared
 * @param is_clear true: clear, false: not clear
 */
void lv_test_fs_clear_open_cb(bool is_clear);

/**
 * Set whether the close callback of the test file system is cleared
 * @param is_clear true: clear, false: not clear
 */
void lv_test_fs_clear_close_cb(bool is_clear);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_TEST*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TEST_FS_H*/


/** @} */
