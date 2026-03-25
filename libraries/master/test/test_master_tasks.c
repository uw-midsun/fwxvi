/************************************************************************************************
 * @file   test_master_tasks.c
 *
 * @brief  Test file for Master Tasks API. Supports 1Hz, 10Hz and 1000Hz scheduling
 *
 * @date   2024-11-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "delay.h"
#include "log.h"
#include "master_tasks.h"
#include "tasks.h"
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */

void setup_test(void) {}

void teardown_test(void) {}

void test_example(void) {
  TEST_ASSERT_TRUE(true);
}
