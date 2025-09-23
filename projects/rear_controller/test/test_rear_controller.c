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
#include "rear_controller.h"
#include "status.h"
#include "gpio.h"

/* Intra-component Headers */
void setup_test(void) {}

void teardown_test(void) {}

void test_example(void) {
  TEST_ASSERT_TRUE(true);
}

//Mock storage for relay tests
static RearControllerStorage test_rear_controller_storage;

TEST_IN_TASK
void test_relay_init(void)(
  TEST_ASSERT_TRUE(test_rear_controller!=nullptr);
  StatusCode status=relay_init(test_rear_controller_storage);
  TEST_ASSET_EQUAL(STATUS_CODE_OK, status);

  //Check relays are all closed
  TEST_ASSERT_TRUE(test_rear_contoller.pos_relay_closed);
  TEST_ASSERT_TRUE(test_rear_contoller.neg_relay_closed);
  TEST_ASSERT_TRUE(test_rear_contoller.solar_relay_closed);

)

TEST_IN_TASK
void test_relay_fault(void){
  StatusCode status=relay_fault();
  TEST_ASSET_EQUAL(STATUS_CODE_OK, status);

  //Check relays are all open
  TEST_ASSERT_FALSE(test_rear_contoller.pos_relay_closed);
  TEST_ASSERT_FALSE(test_rear_contoller.neg_relay_closed);
  TEST_ASSERT_FALSE(test_rear_contoller.solar_relay_closed);
  TEST_ASSERT_FALSE(test_rear_contoller.motor_relay_closed);
}


void test_relay_solar_close(void){
  StatusCode status=relay_solar_close();
  TEST_ASSET_EQUAL(STATUS_CODE_OK, status);
  TEST_ASSERT_TRUE(test_rear_contoller.solar_relay_closed);
}