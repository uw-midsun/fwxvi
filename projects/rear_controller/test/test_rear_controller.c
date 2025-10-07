/************************************************************************************************
 * @file   test_rear_controller.c
 *
 * @brief  Test file for rear_controller
 *
 * @date   2025-05-28
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "rear_controller.h"
#include "relays.h"
#include "status.h"
void setup_test(void) {}

void teardown_test(void) {}

void test_example(void) {
  TEST_ASSERT_TRUE(true);
}


TEST_IN_TASK
void SetupTest(void){
    RearControllerConfig config;
    StatusCode ret = rear_controller_init(&config);
    CHECK_EQUAL(STATUS_CODE_OK, ret);
}

TEST_IN_TASK
void testFault(void){
  StatusCode ret=relay_fault();
  CHECK_EQUAL(STATUS_CODE_OK, ret);
}
TEST_IN_TASK
void test_rear_controller_solar_close_calls_relay_solar_close(void) {
    StatusCode ret = relay_solar_close();
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
    TEST_ASSERT_TRUE(relay_solar_close_called);
}
TEST_IN_TASK
void test_rear_controller_solar_open_calls_relay_solar_open(void) {
    StatusCode ret = relay_solar_open();
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
    TEST_ASSERT_TRUE(relay_solar_open_called);
}
TEST_IN_TASK
void test_rear_controller_motor_close_calls_relay_motor_close(void) {
    StatusCode ret = relay_motor_close();
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
    TEST_ASSERT_TRUE(relay_motor_close_called);
}
TEST_IN_TASK
void test_rear_controller_motor_open_calls_relay_motor_open(void) {
    StatusCode ret = relay_motor_open();
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
    TEST_ASSERT_TRUE(relay_motor_open_called);
}