#pragma once

/************************************************************************************************
 * @file   test_helpers.h
 *
 * @brief  Unity test framework helper library
 *
 * @date   2024-10-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "status.h"
#include "unity.h"

/**
 * @defgroup Miscellaneous
 * @brief    Miscellaneous libraries
 * @{
 */

// General use:
#define TEST_ASSERT_OK(code) TEST_ASSERT_EQUAL(STATUS_CODE_OK, (code))
#define TEST_ASSERT_NOT_OK(code) TEST_ASSERT_NOT_EQUAL(STATUS_CODE_OK, (code))

// Mocking
#define TEST_MOCK(func) __attribute__((used)) __wrap_##func

// Parameterized tests, see test_parameterized.c for usage examples
#define TEST_CASE(...)

#ifndef TEST_PRE_TASK
// define in-task/pre-task tests
#define TEST_PRE_TASK
#define TEST_IN_TASK
#endif

/**
 * @brief Initializes test start and end semaphores
 */
void test_helpers_init_semphr(void);

/**
 * @brief   Gets the semaphore for the test start
 * @return  Value from 0-10
 */
uint8_t test_helpers_start_get_semphr(void);

/**
 * @brief   Gets the semaphore for the test end
 * @return  Value from 0-10
 */
uint8_t test_helpers_end_get_semphr(void);

/**
 * @brief   Gives the test start semaphore
 * @return  STATUS_CODE_OK if the semaphore was given successfully
 *          STATUS_CODE_INTERNAL_ERROR if it fails
 */
StatusCode test_helpers_start_give_semphr(void);

/**
 * @brief   Gives the test end semaphore
 * @return  STATUS_CODE_OK if the semaphore was given successfully
 *          STATUS_CODE_INTERNAL_ERROR if it fails
 */
StatusCode test_helpers_end_give_semphr(void);

/**
 * @brief   Takes the test start semaphore
 * @return  STATUS_CODE_OK if the semaphore was successfully taken
 *          STATUS_CODE_INTERNAL_ERROR if it fails
 */
StatusCode test_helpers_start_take_semphr(void);

/**
 * @brief   Takes the test end semaphore
 * @return  STATUS_CODE_OK if the semaphore was successfully taken
 *          STATUS_CODE_INTERNAL_ERROR if it fails
 */
StatusCode test_helpers_end_take_semphr(void);

/** @} */
