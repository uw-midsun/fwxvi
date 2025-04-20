#pragma once

/************************************************************************************************
 * @file    file_formatter_test.h
 *
 * @brief   File Formatter Test
 *
 * @date    2025-04-20
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>
#include <stdlib.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup file
 * @brief    file Firmware
 * @{{
 */

typedef enum { TEST_ENUM_1, TEST_ENUM_2, NUM_TEST_ENUM } FileFormatterTestEnum;

typedef struct {
  uint8_t data_1;
  uint8_t data_2;
} FileFormatterTestStruct;

FileFormatterTestEnum test_function_foo();
FileFormatterTestEnum test_function_bar();

/** @} */
