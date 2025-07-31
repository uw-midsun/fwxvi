/************************************************************************************************
 * @file    file_formatter_test.c
 *
 * @brief   File Formatter Test
 *
 * @date    2025-04-29
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/
/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "file_formatter_test.h"

/* Intra-component Headers */

FileFormatterTestStruct test_struct = { .data_1 = 0U, .data_2 = 0U };

FileFormatterTestEnum test_function_foo() {
  return TEST_ENUM_1;
}

FileFormatterTestEnum test_function_bar() {
  return TEST_ENUM_2;
}
