#pragma once

/************************************************************************************************
 * @file    lv_test_screenshot_compare.h
 *
 * @brief   Lv Test Screenshot Compare
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup lv_test_screenshot_compare
 * @brief    lv_test_screenshot_compare Firmware
 * @{
 */

#ifndef LV_TEST_SCREENSHOT_COMPARE_H
#define LV_TEST_SCREENSHOT_COMPARE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_TEST && defined(LV_USE_TEST_SCREENSHOT_COMPARE) && LV_USE_TEST_SCREENSHOT_COMPARE

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**
 * Return value of `lv_test_screenshot_compare`
 */
typedef enum {
    /**
     * The screenshot is different than the reference image
     */
    LV_TEST_SCREENSHOT_RESULT_FAILED,

    /**
     * The screenshot is the same as the reference image.
     * It is also returned if `LV_TEST_SCREENSHOT_CREATE_REFERENCE_IMAGE` is enabled
     * and the reference image was missing.
     */
    LV_TEST_SCREENSHOT_RESULT_PASSED,

    /**
     * If `LV_TEST_SCREENSHOT_CREATE_REFERENCE_IMAGE` is not enabled
     * and the reference image is missing.
     */
    LV_TEST_SCREENSHOT_RESULT_NO_REFERENCE_IMAGE,

} lv_test_screenshot_result_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Compare the current content of the test screen with a reference PNG image
 * - If the reference image is not found it will be created automatically from the rendered screen.
 * - If the compare fails an `<image_name>_err.png` file will be created with the rendered content next to the reference image.
 *
 * It requires lodepng.
 *
 * @param fn_ref    path to the reference image. Will be appended to REF_IMGS_PATH if set.
 * @return          An element of `lv_test_screenshot_result_t`
 * @note            This function assumes that the default display is the test display that was created by
 *                  `lv_test_display_create()`
 */
lv_test_screenshot_result_t lv_test_screenshot_compare(const char * fn_ref);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_TEST_SCREENSHOT_COMPARE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TEST_SCREENSHOT_COMPARE_H*/

/** @} */
