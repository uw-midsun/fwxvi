/************************************************************************************************
 * @file   test_state_manager.c
 *
 * @brief  Test file for front controller state manager
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
#include "front_controller_getters.h"
#include "front_controller_state_manager.h"
#include "power_manager.h"

static FrontControllerStorage mock_storage = { 0 };
static uint8_t power_manager_call_count = 0;
static OutputGroup last_output_group = OUTPUT_GROUP_DRIVE;

static SteeringLightState last_light_state = STEERING_LIGHTS_NUM_STATES;
static uint8_t lights_call_count = 0;

static BpsLightState last_bps_light_state = NUM_BPS_LIGHT_STATES;
static uint8_t bps_light_call_count = 0;

StatusCode TEST_MOCK(power_manager_set_output_group)(OutputGroup group, bool enable) {
  (void)enable;
  power_manager_call_count++;
  last_output_group = group;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(front_lights_signal_process_event)(SteeringLightState new_state) {
  lights_call_count++;
  last_light_state = new_state;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(front_lights_signal_set_bps_light)(BpsLightState new_state) {
  bps_light_call_count++;
  last_bps_light_state = new_state;
  return STATUS_CODE_OK;
}

static void prv_reset_mocks(void) {
  power_manager_call_count = 0;
  last_output_group = OUTPUT_GROUP_DRIVE;
  lights_call_count = 0;
  last_light_state = STEERING_LIGHTS_NUM_STATES;
  bps_light_call_count = 0;
  last_bps_light_state = NUM_BPS_LIGHT_STATES;
}

void setup_test(void) {
  mock_storage = (FrontControllerStorage){ 0 };
  g_rx_struct.steering_buttons = 0;
  g_rx_struct.rear_controller_status_triggers = 0;

  front_controller_state_manager_init(&mock_storage);

  // Consume the started=false initialization so subsequent calls process real inputs
  front_controller_update_state_manager_medium_cycle();
  // Second call with brake_state=DISABLED flushes s_brake_state if a previous test left it BRAKING
  front_controller_update_state_manager_medium_cycle();
  prv_reset_mocks();
}

void teardown_test(void) {}

/* ---- init tests ---- */

TEST_IN_TASK
void test_init_with_null_storage_expect_error(void) {
  TEST_ASSERT_NOT_OK(front_controller_state_manager_init(NULL));
}

TEST_IN_TASK
void test_init_sets_neutral_state_expect_success(void) {
  TEST_ASSERT_OK(front_controller_state_manager_init(&mock_storage));
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_NEUTRAL, front_controller_state_manager_get_state());
}

/* ---- step() state transition tests ---- */

TEST_IN_TASK
void test_step_neutral_to_drive_expect_success(void) {
  TEST_ASSERT_OK(front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_DRIVE_REQUEST));
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_DRIVE, front_controller_state_manager_get_state());
  TEST_ASSERT_EQUAL(2, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_DRIVE, last_output_group);
}

TEST_IN_TASK
void test_step_neutral_to_reverse_expect_success(void) {
  TEST_ASSERT_OK(front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_REVERSE_REQUEST));
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_REVERSE, front_controller_state_manager_get_state());
  TEST_ASSERT_EQUAL(2, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_REVERSE, last_output_group);
}

TEST_IN_TASK
void test_step_neutral_to_brake_expect_success(void) {
  TEST_ASSERT_OK(front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_BRAKE_REQUEST));
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_BRAKE, front_controller_state_manager_get_state());
  // enter_state(BRAKE): D_R_INDICATORS(false) + IDLE(true)
  TEST_ASSERT_EQUAL(2, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_IDLE, last_output_group);
}

TEST_IN_TASK
void test_step_neutral_to_regen_expect_success(void) {
  TEST_ASSERT_OK(front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_REGEN_REQUEST));
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_REGEN, front_controller_state_manager_get_state());
  TEST_ASSERT_EQUAL(2, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_IDLE, last_output_group);
}

TEST_IN_TASK
void test_step_neutral_to_fault_expect_success(void) {
  TEST_ASSERT_OK(front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_FAULT));
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_FAULT, front_controller_state_manager_get_state());
  // enter_state(FAULT): IDLE(true) only — no D_R_INDICATORS call
  TEST_ASSERT_EQUAL(1, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_IDLE, last_output_group);
}

TEST_IN_TASK
void test_step_fault_resets_to_neutral_on_reset_event(void) {
  front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_FAULT);
  prv_reset_mocks();

  TEST_ASSERT_OK(front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_RESET));
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_NEUTRAL, front_controller_state_manager_get_state());
  TEST_ASSERT_EQUAL(2, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_IDLE, last_output_group);
}

TEST_IN_TASK
void test_step_fault_ignores_non_reset_events(void) {
  front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_FAULT);
  prv_reset_mocks();

  front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_DRIVE_REQUEST);
  front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_BRAKE_REQUEST);
  front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_REVERSE_REQUEST);

  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_FAULT, front_controller_state_manager_get_state());
  TEST_ASSERT_EQUAL(0, power_manager_call_count);
}

TEST_IN_TASK
void test_step_drive_to_neutral_expect_success(void) {
  front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_DRIVE_REQUEST);
  prv_reset_mocks();

  TEST_ASSERT_OK(front_controller_state_manager_step(FRONT_CONTROLLER_EVENT_IDLE_REQUEST));
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_NEUTRAL, front_controller_state_manager_get_state());
  TEST_ASSERT_EQUAL(2, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_IDLE, last_output_group);
}

/* ---- update_state_manager_medium_cycle() tests ---- */

TEST_IN_TASK
void test_bps_fault_enters_fault_state_expect_success(void) {
  g_rx_struct.rear_controller_status_triggers = 1U;  // bps_fault at bit 0

  TEST_ASSERT_OK(front_controller_update_state_manager_medium_cycle());
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_FAULT, front_controller_state_manager_get_state());
  TEST_ASSERT_EQUAL(1, bps_light_call_count);
  TEST_ASSERT_EQUAL(BPS_LIGHT_ON_STATE, last_bps_light_state);
  // enter_state(FAULT) from NEUTRAL: IDLE(true) only
  TEST_ASSERT_EQUAL(1, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_IDLE, last_output_group);
}

TEST_IN_TASK
void test_bps_fault_persists_no_extra_power_calls_expect_success(void) {
  g_rx_struct.rear_controller_status_triggers = 1U;
  front_controller_update_state_manager_medium_cycle();  // enter fault
  prv_reset_mocks();

  // Second call: already in FAULT — enter_state condition is false, no power_manager call
  front_controller_update_state_manager_medium_cycle();

  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_FAULT, front_controller_state_manager_get_state());
  TEST_ASSERT_EQUAL(0, power_manager_call_count);
  TEST_ASSERT_EQUAL(1, bps_light_call_count);
  TEST_ASSERT_EQUAL(BPS_LIGHT_ON_STATE, last_bps_light_state);
}

TEST_IN_TASK
void test_bps_fault_cleared_resets_to_neutral_expect_success(void) {
  g_rx_struct.rear_controller_status_triggers = 1U;
  front_controller_update_state_manager_medium_cycle();  // enter fault
  prv_reset_mocks();

  g_rx_struct.rear_controller_status_triggers = 0U;
  TEST_ASSERT_OK(front_controller_update_state_manager_medium_cycle());
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_NEUTRAL, front_controller_state_manager_get_state());
  TEST_ASSERT_EQUAL(1, bps_light_call_count);
  TEST_ASSERT_EQUAL(BPS_LIGHT_OFF_STATE, last_bps_light_state);
  // enter_state(NEUTRAL) from FAULT: D_R_INDICATORS(false) + IDLE(true)
  TEST_ASSERT_EQUAL(2, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_IDLE, last_output_group);
}

TEST_IN_TASK
void test_drive_request_with_precharge_complete_expect_success(void) {
  g_rx_struct.rear_controller_status_triggers = (1U << 23);         // precharge_complete at bit 23
  g_rx_struct.steering_buttons = VEHICLE_DRIVE_STATE_DRIVE & 0x3U;  // drive_state bits [1:0]

  TEST_ASSERT_OK(front_controller_update_state_manager_medium_cycle());
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_DRIVE, front_controller_state_manager_get_state());
  // enter_state(DRIVE): D_R_INDICATORS(false) + DRIVE(true)
  TEST_ASSERT_EQUAL(2, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_DRIVE, last_output_group);
}

TEST_IN_TASK
void test_drive_request_without_precharge_expect_no_transition(void) {
  g_rx_struct.rear_controller_status_triggers = 0U;  // precharge not complete
  g_rx_struct.steering_buttons = VEHICLE_DRIVE_STATE_DRIVE & 0x3U;

  TEST_ASSERT_OK(front_controller_update_state_manager_medium_cycle());
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_NEUTRAL, front_controller_state_manager_get_state());
  TEST_ASSERT_EQUAL(0, power_manager_call_count);
}

TEST_IN_TASK
void test_reverse_request_with_precharge_complete_expect_success(void) {
  g_rx_struct.rear_controller_status_triggers = (1U << 23);
  g_rx_struct.steering_buttons = VEHICLE_DRIVE_STATE_REVERSE & 0x3U;  // reverse = 3 = 0b11

  TEST_ASSERT_OK(front_controller_update_state_manager_medium_cycle());
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_REVERSE, front_controller_state_manager_get_state());
  TEST_ASSERT_EQUAL(2, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_REVERSE, last_output_group);
}

TEST_IN_TASK
void test_neutral_request_from_drive_expect_success(void) {
  g_rx_struct.rear_controller_status_triggers = (1U << 23);
  g_rx_struct.steering_buttons = VEHICLE_DRIVE_STATE_DRIVE & 0x3U;
  front_controller_update_state_manager_medium_cycle();  // enter drive
  prv_reset_mocks();

  g_rx_struct.steering_buttons = VEHICLE_DRIVE_STATE_NEUTRAL & 0x3U;  // neutral = 1
  TEST_ASSERT_OK(front_controller_update_state_manager_medium_cycle());
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_NEUTRAL, front_controller_state_manager_get_state());
  TEST_ASSERT_EQUAL(2, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_IDLE, last_output_group);
}

TEST_IN_TASK
void test_horn_enabled_expect_success(void) {
  g_rx_struct.steering_buttons = (1U << 6);  // horn_enabled at bit 6

  TEST_ASSERT_OK(front_controller_update_state_manager_medium_cycle());
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_NEUTRAL, front_controller_state_manager_get_state());
  // power_manager called once for HORN
  TEST_ASSERT_EQUAL(1, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_HORN, last_output_group);
}

TEST_IN_TASK
void test_horn_already_enabled_no_extra_call_expect_success(void) {
  g_rx_struct.steering_buttons = (1U << 6);
  front_controller_update_state_manager_medium_cycle();  // enable horn
  prv_reset_mocks();

  // Horn still on — is_horn_enabled=true, no state change, no call
  front_controller_update_state_manager_medium_cycle();
  TEST_ASSERT_EQUAL(0, power_manager_call_count);
}

TEST_IN_TASK
void test_horn_disabled_after_enabled_expect_success(void) {
  g_rx_struct.steering_buttons = (1U << 6);
  front_controller_update_state_manager_medium_cycle();  // enable horn
  prv_reset_mocks();

  g_rx_struct.steering_buttons = 0U;
  front_controller_update_state_manager_medium_cycle();

  TEST_ASSERT_EQUAL(1, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_HORN, last_output_group);
}

TEST_IN_TASK
void test_horn_already_disabled_no_call_expect_success(void) {
  g_rx_struct.steering_buttons = 0U;

  TEST_ASSERT_OK(front_controller_update_state_manager_medium_cycle());
  TEST_ASSERT_EQUAL(0, power_manager_call_count);
}

TEST_IN_TASK
void test_lights_all_valid_states_expect_success(void) {
  SteeringLightState valid_states[STEERING_LIGHTS_NUM_STATES] = {
    STEERING_LIGHTS_OFF_STATE,
    STEERING_LIGHTS_LEFT_STATE,
    STEERING_LIGHTS_RIGHT_STATE,
    STEERING_LIGHTS_HAZARD_STATE,
  };

  for (uint8_t i = 0; i < STEERING_LIGHTS_NUM_STATES; i++) {
    lights_call_count = 0;
    // lights at bits [3:2]
    g_rx_struct.steering_buttons = (valid_states[i] & 0x3U) << 2;

    TEST_ASSERT_OK(front_controller_update_state_manager_medium_cycle());
    TEST_ASSERT_EQUAL(1, lights_call_count);
    TEST_ASSERT_EQUAL(valid_states[i], last_light_state);
  }
}

TEST_IN_TASK
void test_brake_engaged_without_regen_expect_brake_state(void) {
  mock_storage.brake_state = BRAKE_STATE_BRAKING;

  TEST_ASSERT_OK(front_controller_update_state_manager_medium_cycle());
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_BRAKE, front_controller_state_manager_get_state());
  // BRAKE_LIGHTS(true) + D_R_INDICATORS(false) + IDLE(true)
  TEST_ASSERT_EQUAL(3, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_IDLE, last_output_group);
}

TEST_IN_TASK
void test_brake_engaged_with_regen_expect_regen_state(void) {
  mock_storage.brake_state = BRAKE_STATE_BRAKING;
  g_rx_struct.steering_buttons = (1U << 7);  // regen_enabled at bit 7

  TEST_ASSERT_OK(front_controller_update_state_manager_medium_cycle());
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_REGEN, front_controller_state_manager_get_state());
  TEST_ASSERT_EQUAL(3, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_IDLE, last_output_group);
}

TEST_IN_TASK
void test_brake_released_turns_off_brake_lights_expect_success(void) {
  mock_storage.brake_state = BRAKE_STATE_BRAKING;
  front_controller_update_state_manager_medium_cycle();  // engage brake
  prv_reset_mocks();

  mock_storage.brake_state = BRAKE_STATE_DISABLED;
  TEST_ASSERT_OK(front_controller_update_state_manager_medium_cycle());
  // Only BRAKE_LIGHTS(false) — no state transition
  TEST_ASSERT_EQUAL(1, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_BRAKE_LIGHTS, last_output_group);
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_BRAKE, front_controller_state_manager_get_state());
}

TEST_IN_TASK
void test_drive_and_horn_enabled_expect_success(void) {
  g_rx_struct.rear_controller_status_triggers = (1U << 23);
  g_rx_struct.steering_buttons = (VEHICLE_DRIVE_STATE_DRIVE & 0x3U) | (1U << 6);

  TEST_ASSERT_OK(front_controller_update_state_manager_medium_cycle());
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE_STATE_DRIVE, front_controller_state_manager_get_state());
  // enter_state(DRIVE): D_R_INDICATORS(false) + DRIVE(true), then HORN(true) last
  TEST_ASSERT_EQUAL(3, power_manager_call_count);
  TEST_ASSERT_EQUAL(OUTPUT_GROUP_HORN, last_output_group);
}
