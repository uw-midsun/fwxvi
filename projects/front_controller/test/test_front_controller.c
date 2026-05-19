/************************************************************************************************
 * @file   test_front_controller.c
 *
 * @brief  Unit tests for front_controller module
 *
 * @date   2026-05-15
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "front_controller.h"

static FrontControllerStorage mock_storage = { 0 };
static FrontControllerConfig mock_config = { 0 };

void setup_test(void) {}

void teardown_test(void) {}

TEST_IN_TASK
void test_front_controller_init_null_storage_returns_error(void) {
  TEST_ASSERT_NOT_OK(front_controller_init(NULL, &mock_config, NULL));
}

TEST_IN_TASK
void test_front_controller_init_null_config_returns_error(void) {
  TEST_ASSERT_NOT_OK(front_controller_init(&mock_storage, NULL, NULL));
}
