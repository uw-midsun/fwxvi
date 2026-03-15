#pragma once

/************************************************************************************************
 * @file    lv_test.h
 *
 * @brief   Lv Test
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"
#include "lv_test_display.h"
#include "lv_test_fs.h"
#include "lv_test_helpers.h"
#include "lv_test_indev.h"
#include "lv_test_indev_gesture.h"
#include "lv_test_screenshot_compare.h"

/* Intra-component Headers */

/**
 * @defgroup lv_test
 * @brief    lv_test Firmware
 * @{
 */

#ifndef LV_TEST_H
#define LV_TEST_H

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

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE TEST*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TEST_H*/

/** @} */
