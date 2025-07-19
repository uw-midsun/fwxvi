/************************************************************************************************
 * @file   test_steering.c
 *
 * @brief  Test file for steering
 *
 * @date   2025-07-09
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "light_signal_manager.h"

void setup_test(void) {}

void teardown_test(void) {}

void test_example(void) {
  TEST_ASSERT_TRUE(true);
}
TEST_IN_TASK
void test_left(void) {
  lights_signal_manager_init();

  // Initial state should be OFF
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_STATE_OFF, lights_signal_manager_get_state());

  // No request yet, should stay OFF
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_STATE_OFF, lights_signal_manager_get_state());

  // Request LEFT, should transition to LEFT
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_LEFT);
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_STATE_LEFT, lights_signal_manager_get_state());

  // Request LEFT again — should remain LEFT
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_LEFT);
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_STATE_LEFT, lights_signal_manager_get_state());

  // Turn off signal
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_OFF);
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_STATE_OFF, lights_signal_manager_get_state());

  // Rapid change: LEFT then RIGHT — only RIGHT should be applied
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_LEFT);
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_RIGHT);
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_STATE_RIGHT, lights_signal_manager_get_state());
}

TEST_IN_TASK
void test_right(void) {
  lights_signal_manager_init();

  // Initial state should be OFF
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_STATE_OFF, lights_signal_manager_get_state());

  // No request yet, should stay OFF
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_STATE_OFF, lights_signal_manager_get_state());

  // Request RIGHT, should transition to RIGHT
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_RIGHT);
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_REQUEST_RIGHT, lights_signal_manager_get_state());

  // Request RIGHT again — should remain RIGHT
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_RIGHT);
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_REQUEST_RIGHT, lights_signal_manager_get_state());

  // Turn off signal
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_OFF);
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_STATE_OFF, lights_signal_manager_get_state());

  // Rapid change: LEFT then RIGHT — only RIGHT should be applied
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_RIGHT);
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_LEFT);
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_REQUEST_LEFT, lights_signal_manager_get_state());
}

TEST_IN_TASK
void test_hazard(void) {
  lights_signal_manager_init();

  // Initial state should be OFF
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_STATE_OFF, lights_signal_manager_get_state());

  // No request yet, should stay OFF
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_STATE_OFF, lights_signal_manager_get_state());

  // Request RIGHT, should transition to RIGHT
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_RIGHT);
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_REQUEST_RIGHT, lights_signal_manager_get_state());

  // Request HAZARD — should transition to HAZARD
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_HAZARD);
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_REQUEST_HAZARD, lights_signal_manager_get_state());

  // Request HAZARD again — should stay the same
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_HAZARD);
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_REQUEST_HAZARD, lights_signal_manager_get_state());

  // Request LEFT while on Hazard - should not transition
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_LEFT);
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_REQUEST_HAZARD, lights_signal_manager_get_state());

  // Request RIGHT while on Hazard - should not transition
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_RIGHT);
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_REQUEST_HAZARD, lights_signal_manager_get_state());

  // Turn off signal
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_OFF);
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_STATE_OFF, lights_signal_manager_get_state());

  // Request LEFT - should transition
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_LEFT);
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_REQUEST_LEFT, lights_signal_manager_get_state());

  // Request RIGHT - should transition
  lights_signal_manager_request(LIGHTS_SIGNAL_REQUEST_RIGHT);
  lights_signal_manager_update();
  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_REQUEST_RIGHT, lights_signal_manager_get_state());
}
