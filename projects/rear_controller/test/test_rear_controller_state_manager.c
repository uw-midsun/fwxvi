/************************************************************************************************
 * @file    test_rear_controller_state_manager.c
 *
 * @brief   Test file for Rear Controller State Manager
 *
 * @date    2025-10-24
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/
/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "rear_controller_state_manager.h"
#include "relays.h"

static RearControllerStorage s_test_rear_storage;

static bool s_reset_called = false;
static bool s_close_pos_called = false;
static bool s_close_neg_called = false;
static bool s_close_motor_called = false;
static bool s_close_solar_called = false;

StatusCode TEST_MOCK(relays_reset)(void) {
  s_reset_called = true;
  return STATUS_CODE_OK;
}
StatusCode TEST_MOCK(relays_close_pos)(void) {
  s_close_pos_called = true;
  return STATUS_CODE_OK;
}
StatusCode TEST_MOCK(relays_close_neg)(void) {
  s_close_neg_called = true;
  return STATUS_CODE_OK;
}
StatusCode TEST_MOCK(relays_close_motor)(void) {
  s_close_motor_called = true;
  return STATUS_CODE_OK;
}
StatusCode TEST_MOCK(relays_close_solar)(void) {
  s_close_solar_called = true;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  s_reset_called = false;
  s_close_pos_called = false;
  s_close_neg_called = false;
  s_close_motor_called = false;
  s_close_solar_called = false;
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, rear_controller_state_manager_init(&s_test_rear_storage));
}

void teardown_test(void) {}

static void reset_mock_flags(void) {
  s_reset_called = false;
  s_close_pos_called = false;
  s_close_neg_called = false;
  s_close_motor_called = false;
  s_close_solar_called = false;
}

TEST_IN_TASK
void test_initialization_starts_in_init_state(void) {
  TEST_ASSERT_EQUAL(REAR_CONTROLLER_STATE_INIT, rear_controller_state_manager_get_state());
  TEST_ASSERT_TRUE(s_reset_called);
}

TEST_IN_TASK
void test_init_to_precharge_transition(void) {
  reset_mock_flags();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_INIT_COMPLETE));
  TEST_ASSERT_EQUAL(REAR_CONTROLLER_STATE_PRECHARGE, rear_controller_state_manager_get_state());
  TEST_ASSERT_TRUE(s_close_pos_called);
}

TEST_IN_TASK
void test_precharge_to_idle_transition(void) {
  /* Enter PRECHARGE first */
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_INIT_COMPLETE);
  reset_mock_flags();

  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_PRECHARGE_SUCCESS);
  TEST_ASSERT_EQUAL(REAR_CONTROLLER_STATE_IDLE, rear_controller_state_manager_get_state());
  TEST_ASSERT_TRUE(s_close_neg_called);
}

TEST_IN_TASK
void test_precharge_to_fault_on_fail(void) {
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_INIT_COMPLETE);
  reset_mock_flags();

  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_PRECHARGE_FAIL);
  TEST_ASSERT_EQUAL(REAR_CONTROLLER_STATE_FAULT, rear_controller_state_manager_get_state());
  TEST_ASSERT_TRUE(s_reset_called);
}

TEST_IN_TASK
void test_idle_to_drive_transition(void) {
  /* Move to IDLE first */
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_INIT_COMPLETE);
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_PRECHARGE_SUCCESS);
  reset_mock_flags();

  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_DRIVE_REQUEST);
  TEST_ASSERT_EQUAL(REAR_CONTROLLER_STATE_DRIVE, rear_controller_state_manager_get_state());
  TEST_ASSERT_TRUE(s_close_motor_called);
  TEST_ASSERT_TRUE(s_close_solar_called);
}

TEST_IN_TASK
void test_idle_to_charge_transition(void) {
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_INIT_COMPLETE);
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_PRECHARGE_SUCCESS);
  reset_mock_flags();

  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_CHARGE_REQUEST);
  TEST_ASSERT_EQUAL(REAR_CONTROLLER_STATE_CHARGE, rear_controller_state_manager_get_state());
  TEST_ASSERT_TRUE(s_close_motor_called);
}

TEST_IN_TASK
void test_drive_to_init_on_neutral(void) {
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_INIT_COMPLETE);
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_PRECHARGE_SUCCESS);
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_DRIVE_REQUEST);
  reset_mock_flags();

  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_NEUTRAL_REQUEST);
  TEST_ASSERT_EQUAL(REAR_CONTROLLER_STATE_INIT, rear_controller_state_manager_get_state());
  TEST_ASSERT_TRUE(s_reset_called);
}

TEST_IN_TASK
void test_charge_to_init_on_charger_removed(void) {
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_INIT_COMPLETE);
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_PRECHARGE_SUCCESS);
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_CHARGE_REQUEST);
  reset_mock_flags();

  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_CHARGER_REMOVED);
  TEST_ASSERT_EQUAL(REAR_CONTROLLER_STATE_INIT, rear_controller_state_manager_get_state());
  TEST_ASSERT_TRUE(s_reset_called);
}

TEST_IN_TASK
void test_fault_and_reset_behavior(void) {
  /* Trigger a fault from IDLE */
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_INIT_COMPLETE);
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_PRECHARGE_SUCCESS);
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_FAULT);
  TEST_ASSERT_EQUAL(REAR_CONTROLLER_STATE_FAULT, rear_controller_state_manager_get_state());

  /* Now reset system */
  reset_mock_flags();
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_RESET);
  TEST_ASSERT_EQUAL(REAR_CONTROLLER_STATE_INIT, rear_controller_state_manager_get_state());
  TEST_ASSERT_TRUE(s_reset_called);
}
