/************************************************************************************************
 * @file   test_power_state.c
 *
 * @brief  Test file for power state manager
 *
 * @date   2025-11-26
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "front_controller_getters.h"
#include "power_control_outputs.h"
#include "power_state.h"

static FrontControllerStorage mock_storage = { 0 };
static uint8_t power_control_set_output_group_call_count = 0;

StatusCode TEST_MOCK(power_control_set_output_group)(OutputGroup group, bool enable) {
  (void)group;
  (void)enable;
  power_control_set_output_group_call_count++;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  power_state_init(&mock_storage);
  g_rx_struct.rear_controller_status_bps_fault = 0;
  g_rx_struct.battery_stats_B_motor_precharge_complete = 0;
  g_rx_struct.steering_buttons_drive_state = 0;
  power_control_set_output_group_call_count = 0;
}

void teardown_test(void) {}

TEST_IN_TASK
void test_bps_fault_with_valid_params_expect_success(void) {
  g_rx_struct.rear_controller_status_bps_fault = 1;
  StatusCode ret = power_state_set_load_switches();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL(POWER_STATE_FAULT, mock_storage.current_power_state);
  TEST_ASSERT_EQUAL(2, power_control_set_output_group_call_count);
}

TEST_IN_TASK
void test_bps_fault_with_already_fault_expect_success(void) {
  g_rx_struct.rear_controller_status_bps_fault = 1;
  StatusCode ret = power_state_set_load_switches();
  mock_storage.current_power_state = POWER_STATE_FAULT;

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL(POWER_STATE_FAULT, mock_storage.current_power_state);
  TEST_ASSERT_EQUAL(0, power_control_set_output_group_call_count);
}

TEST_IN_TASK
void test_drive_with_valid_params_expect_success(void) {
  VehicleDriveState validStates[3] = { VEHICLE_DRIVE_STATE_CRUISE, VEHICLE_DRIVE_STATE_DRIVE, VEHICLE_DRIVE_STATE_REVERSE };
  g_rx_struct.battery_stats_B_motor_precharge_complete = 1;

  for (uint8_t i = 0; i < 3; i++) {
    mock_storage.current_power_state = POWER_STATE_FAULT;
    power_control_set_output_group_call_count = 0;
    g_rx_struct.steering_buttons_drive_state = validStates[i];

    StatusCode ret = power_state_set_load_switches();

    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
    TEST_ASSERT_EQUAL(POWER_STATE_ENGAGED, mock_storage.current_power_state);
    TEST_ASSERT_EQUAL(1, power_control_set_output_group_call_count);
  }
}

TEST_IN_TASK
void test_drive_with_already_driving_expect_success(void) {
  VehicleDriveState validStates[3] = { VEHICLE_DRIVE_STATE_CRUISE, VEHICLE_DRIVE_STATE_DRIVE, VEHICLE_DRIVE_STATE_REVERSE };
  g_rx_struct.battery_stats_B_motor_precharge_complete = 1;
  mock_storage.current_power_state = POWER_STATE_ENGAGED;

  for (uint8_t i = 0; i < 3; i++) {
    power_control_set_output_group_call_count = 0;
    g_rx_struct.steering_buttons_drive_state = validStates[i];

    StatusCode ret = power_state_set_load_switches();

    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
    TEST_ASSERT_EQUAL(POWER_STATE_ENGAGED, mock_storage.current_power_state);
    TEST_ASSERT_EQUAL(0, power_control_set_output_group_call_count);
  }
}

TEST_IN_TASK
void test_drive_with_precharge_incomplete_expect_no_change(void) {
  VehicleDriveState validStates[3] = { VEHICLE_DRIVE_STATE_CRUISE, VEHICLE_DRIVE_STATE_DRIVE, VEHICLE_DRIVE_STATE_REVERSE };
  g_rx_struct.battery_stats_B_motor_precharge_complete = 0;
  mock_storage.current_power_state = POWER_STATE_IDLE;

  for (uint8_t i = 0; i < 3; i++) {
    power_control_set_output_group_call_count = 0;
    g_rx_struct.steering_buttons_drive_state = validStates[i];

    StatusCode ret = power_state_set_load_switches();

    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
    TEST_ASSERT_EQUAL(POWER_STATE_IDLE, mock_storage.current_power_state);
    TEST_ASSERT_EQUAL(0, power_control_set_output_group_call_count);
  }
}

TEST_IN_TASK
void test_idle_with_valid_params_expect_success(void) {
  g_rx_struct.steering_buttons_drive_state = VEHICLE_DRIVE_STATE_NEUTRAL;
  mock_storage.current_power_state = POWER_STATE_ENGAGED;
  StatusCode ret = power_state_set_load_switches();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL(POWER_STATE_IDLE, mock_storage.current_power_state);
  TEST_ASSERT_EQUAL(2, power_control_set_output_group_call_count);
}

TEST_IN_TASK
void test_idle_with_already_idle_expect_success(void) {
  g_rx_struct.steering_buttons_drive_state = VEHICLE_DRIVE_STATE_NEUTRAL;
  mock_storage.current_power_state = POWER_STATE_IDLE;
  StatusCode ret = power_state_set_load_switches();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL(POWER_STATE_IDLE, mock_storage.current_power_state);
  TEST_ASSERT_EQUAL(0, power_control_set_output_group_call_count);
}
