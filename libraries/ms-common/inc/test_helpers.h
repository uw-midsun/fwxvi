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

/** @} */
