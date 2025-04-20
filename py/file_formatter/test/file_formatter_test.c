#include "file_formatter_test.h"

#include <string.h>

FileFormatterTestStruct test_struct = { .data_1 = 0U, .data_2 = 0U };

FileFormatterTestEnum test_function_foo() {
  return TEST_ENUM_1;
}

FileFormatterTestEnum test_function_bar() {
  return TEST_ENUM_2;
}
