/************************************************************************************************
 * @file   test_state_manager.c
 *
 * @brief  Test file for front controller state manager
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
#include "front_controller_state_manager.h"
#include "power_manager.h"

static FrontControllerStorage mock_storage = { 0 };
static uint8_t power_manager_set_output_group_call_count = 0;
static OutputGroup current_output_group = OUTPUT_GROUP_ACTIVE;

static SteeringLightState current_light_state = NUM_STEERING_LIGHTS;
static uint8_t front_lights_signal_process_event_call_count = 0;

StatusCode TEST_MOCK(power_manager_set_output_group)(OutputGroup group, bool enable) {
  (void)enable;
  power_manager_set_output_group_call_count++;
  current_output_group = group;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(front_lights_signal_process_event)(SteeringLightState new_state) {
  front_lights_signal_process_event_call_count++;
  current_light_state = new_state;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  front_controller_state_manager_init(&mock_storage);
  g_rx_struct.rear_controller_status_bps_fault = 0;
  g_rx_struct.battery_stats_B_motor_precharge_complete = 0;
  g_rx_struct.steering_buttons_drive_state = 0;
  g_rx_struct.steering_buttons_horn_enabled = 0;
  g_rx_struct.steering_buttons_lights = 0;

  power_manager_set_output_group_call_count = 0;
  front_lights_signal_process_event_call_count = 0;
  current_light_state = NUM_STEERING_LIGHTS;
  current_output_group = OUTPUT_GROUP_ACTIVE;
}

void teardown_test(void) {}

TEST_IN_TASK
void test_bps_fault_with_valid_params_expect_success(void) {
  g_rx_struct.rear_controller_status_bps_fault = 1;
  StatusCode ret = front_controller_update_state_manager_medium_cycle();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  FrontControllerState curr_state = front_controller_state_manager_get_state();
  TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_FAULT, curr_state);
  TEST_ASSERT_EQUAL(2, power_manager_set_output_group_call_count);
  TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_HAZARD_LIGHTS);
}

TEST_IN_TASK
void test_bps_fault_with_already_fault_expect_success(void) {
  g_rx_struct.rear_controller_status_bps_fault = 1;
  StatusCode ret = front_controller_update_state_manager_medium_cycle();
  FrontControllerState curr_state = front_controller_state_manager_get_state();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_FAULT, curr_state);
  TEST_ASSERT_EQUAL(2, power_manager_set_output_group_call_count);
  TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_HAZARD_LIGHTS);

  ret = front_controller_update_state_manager_medium_cycle();
  curr_state = front_controller_state_manager_get_state();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_FAULT, curr_state);
  TEST_ASSERT_EQUAL(2, power_manager_set_output_group_call_count);  // should not increase from second call
  TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_HAZARD_LIGHTS);
}

TEST_IN_TASK
void test_drive_with_valid_params_expect_success(void) {
  VehicleDriveState validStates[3] = { VEHICLE_DRIVE_STATE_CRUISE, VEHICLE_DRIVE_STATE_DRIVE, VEHICLE_DRIVE_STATE_REVERSE };
  g_rx_struct.rear_controller_status_bps_fault = 0;
  g_rx_struct.battery_stats_B_motor_precharge_complete = 1;

  for (uint8_t i = 0; i < 3; i++) {
    power_manager_set_output_group_call_count = 0;
    g_rx_struct.steering_buttons_drive_state = validStates[i];

    StatusCode ret = front_controller_update_state_manager_medium_cycle();
    FrontControllerState curr_state = front_controller_state_manager_get_state();

    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
    TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_ENGAGED, curr_state);
    TEST_ASSERT_EQUAL(1, power_manager_set_output_group_call_count);
    TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_ALL);

    g_rx_struct.steering_buttons_drive_state = VEHICLE_DRIVE_STATE_NEUTRAL;
    ret = front_controller_update_state_manager_medium_cycle();
    curr_state = front_controller_state_manager_get_state();

    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
    TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_IDLE, curr_state);
    TEST_ASSERT_EQUAL(3, power_manager_set_output_group_call_count);
    TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_ACTIVE);
  }
}

TEST_IN_TASK
void test_drive_with_already_driving_expect_success(void) {
  VehicleDriveState validStates[3] = { VEHICLE_DRIVE_STATE_CRUISE, VEHICLE_DRIVE_STATE_DRIVE, VEHICLE_DRIVE_STATE_REVERSE };
  g_rx_struct.battery_stats_B_motor_precharge_complete = 1;

  for (uint8_t i = 0; i < 3; i++) {
    g_rx_struct.steering_buttons_drive_state = validStates[i];

    StatusCode ret = front_controller_update_state_manager_medium_cycle();
    FrontControllerState curr_state = front_controller_state_manager_get_state();

    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
    TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_ENGAGED, curr_state);
    TEST_ASSERT_EQUAL(1, power_manager_set_output_group_call_count);
    TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_ALL);
  }
}

TEST_IN_TASK
void test_drive_with_precharge_incomplete_expect_no_change(void) {
  VehicleDriveState validStates[3] = { VEHICLE_DRIVE_STATE_CRUISE, VEHICLE_DRIVE_STATE_DRIVE, VEHICLE_DRIVE_STATE_REVERSE };
  g_rx_struct.battery_stats_B_motor_precharge_complete = 0;

  for (uint8_t i = 0; i < 3; i++) {
    g_rx_struct.steering_buttons_drive_state = validStates[i];

    StatusCode ret = front_controller_update_state_manager_medium_cycle();
    FrontControllerState curr_state = front_controller_state_manager_get_state();

    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
    TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_IDLE, curr_state);
    TEST_ASSERT_EQUAL(0, power_manager_set_output_group_call_count);
    TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_ACTIVE);
  }
}

TEST_IN_TASK
void test_idle_with_valid_params_expect_success(void) {
  g_rx_struct.rear_controller_status_bps_fault = 0;
  g_rx_struct.steering_buttons_drive_state = VEHICLE_DRIVE_STATE_DRIVE;
  g_rx_struct.battery_stats_B_motor_precharge_complete = 1;
  StatusCode ret = front_controller_update_state_manager_medium_cycle();
  FrontControllerState curr_state = front_controller_state_manager_get_state();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_ENGAGED, curr_state);
  TEST_ASSERT_EQUAL(1, power_manager_set_output_group_call_count);
  TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_ALL);

  g_rx_struct.steering_buttons_drive_state = VEHICLE_DRIVE_STATE_NEUTRAL;
  ret = front_controller_update_state_manager_medium_cycle();
  curr_state = front_controller_state_manager_get_state();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_IDLE, curr_state);
  TEST_ASSERT_EQUAL(3, power_manager_set_output_group_call_count);
  TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_ACTIVE);
}

TEST_IN_TASK
void test_idle_with_already_idle_expect_success(void) {
  g_rx_struct.rear_controller_status_bps_fault = 0;
  g_rx_struct.steering_buttons_drive_state = VEHICLE_DRIVE_STATE_NEUTRAL;
  StatusCode ret = front_controller_update_state_manager_medium_cycle();
  FrontControllerState curr_state = front_controller_state_manager_get_state();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_IDLE, curr_state);
  TEST_ASSERT_EQUAL(0, power_manager_set_output_group_call_count);
  TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_ACTIVE);
}

TEST_IN_TASK
void test_horns_enabled_expect_success(void) {
  g_rx_struct.steering_buttons_horn_enabled = 1;
  StatusCode ret = front_controller_update_state_manager_medium_cycle();
  FrontControllerState curr_state = front_controller_state_manager_get_state();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_IDLE, curr_state);
  TEST_ASSERT_EQUAL(1, power_manager_set_output_group_call_count);
  TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_HORN);
}

TEST_IN_TASK
void test_horns_enabled_with_horn_already_enabled_expect_no_change(void) {
  g_rx_struct.steering_buttons_horn_enabled = 1;
  StatusCode ret = front_controller_update_state_manager_medium_cycle();
  FrontControllerState curr_state = front_controller_state_manager_get_state();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_IDLE, curr_state);
  TEST_ASSERT_EQUAL(1, power_manager_set_output_group_call_count);

  ret = front_controller_update_state_manager_medium_cycle();
  curr_state = front_controller_state_manager_get_state();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_IDLE, curr_state);
  TEST_ASSERT_EQUAL(1, power_manager_set_output_group_call_count);
  TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_HORN);
}

TEST_IN_TASK
void test_horns_disabled_expect_success(void) {
  g_rx_struct.steering_buttons_horn_enabled = 1;
  StatusCode ret = front_controller_update_state_manager_medium_cycle();
  FrontControllerState curr_state = front_controller_state_manager_get_state();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_IDLE, curr_state);
  TEST_ASSERT_EQUAL(1, power_manager_set_output_group_call_count);
  TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_HORN);

  g_rx_struct.steering_buttons_horn_enabled = 0;
  ret = front_controller_update_state_manager_medium_cycle();
  curr_state = front_controller_state_manager_get_state();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_IDLE, curr_state);
  TEST_ASSERT_EQUAL(2, power_manager_set_output_group_call_count);
  TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_HORN);
}

TEST_IN_TASK
void test_horns_disabled_with_horns_already_disabled_expect_no_change(void) {
  g_rx_struct.steering_buttons_horn_enabled = 0;
  StatusCode ret = front_controller_update_state_manager_medium_cycle();
  FrontControllerState curr_state = front_controller_state_manager_get_state();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_IDLE, curr_state);
  TEST_ASSERT_EQUAL(0, power_manager_set_output_group_call_count);
  TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_ACTIVE);
}

TEST_IN_TASK
void test_lights_expect_success(void) {
  SteeringLightState validStates[NUM_STEERING_LIGHTS] = { STEERING_LIGHTS_OFF_STATE, STEERING_LIGHTS_LEFT_STATE, STEERING_LIGHTS_RIGHT_STATE, STEERING_LIGHTS_HAZARD_STATE };

  for (uint8_t i = 0; i < NUM_STEERING_LIGHTS; i++) {
    front_lights_signal_process_event_call_count = 0;
    g_rx_struct.steering_buttons_lights = validStates[i];
    StatusCode ret = front_controller_update_state_manager_medium_cycle();
    FrontControllerState curr_state = front_controller_state_manager_get_state();

    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
    TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_IDLE, curr_state);
    TEST_ASSERT_EQUAL(0, power_manager_set_output_group_call_count);

    TEST_ASSERT_EQUAL(1, front_lights_signal_process_event_call_count);
    TEST_ASSERT_EQUAL(validStates[i], current_light_state);
    TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_ACTIVE);
  }
}

TEST_IN_TASK
void test_drive_and_horn_enabled_expect_success(void) {
  g_rx_struct.rear_controller_status_bps_fault = 0;
  g_rx_struct.battery_stats_B_motor_precharge_complete = 1;
  g_rx_struct.steering_buttons_drive_state = VEHICLE_DRIVE_STATE_DRIVE;
  g_rx_struct.steering_buttons_horn_enabled = 1;

  StatusCode ret = front_controller_update_state_manager_medium_cycle();
  FrontControllerState curr_state = front_controller_state_manager_get_state();

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL(FRONT_CONTROLLER_STATE_ENGAGED, curr_state);
  TEST_ASSERT_EQUAL(2, power_manager_set_output_group_call_count);
  TEST_ASSERT_EQUAL(current_output_group, OUTPUT_GROUP_HORN);
}
