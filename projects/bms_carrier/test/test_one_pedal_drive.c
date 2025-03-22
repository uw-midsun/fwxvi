/************************************************************************************************
 * @file   test_one_pedal_drive.c
 *
 * @brief  Test file for one pedal drive
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "delay.h"
#include "log.h"
#include "tasks.h"
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "bms_carrier.h"
#include "bms_hw_defs.h"
#include "motor_can.h"
#include "one_pedal_drive.h"

static BmsStorage mock_bms_storage = { .bms_config = { .series_count = NUM_SERIES_CELLS, .parallel_count = NUM_PARALLEL_CELLS, .pack_capacity = PACK_CAPACITY_MAH } };

void setup_test(void) {}

void teardown_test(void) {}

TEST_IN_TASK
void test_clamp_float(void) {
  float test_value = 0.5f;

  TEST_ASSERT_FLOAT_WITHIN(0.5f, 0.0f, clamp_float(test_value));
}

TEST_IN_TASK
void test_opd_thresh_1kmh(void) {
  float test_speed = 1.0f;
  float test_answer = (test_speed * COASTING_THRESHOLD_SCALE);

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_threshold(test_speed));
}

TEST_IN_TASK
void test_opd_thresh_2kmh(void) {
  float test_speed = 2.0f;
  float test_answer = (test_speed * COASTING_THRESHOLD_SCALE);

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_threshold(test_speed));
}

TEST_IN_TASK
void test_opd_thresh_3kmh(void) {
  float test_speed = 3.0f;
  float test_answer = (test_speed * COASTING_THRESHOLD_SCALE);

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_threshold(test_speed));
}

TEST_IN_TASK
void test_opd_thresh_4kmh(void) {
  float test_speed = 4.0f;
  float test_answer = (test_speed * COASTING_THRESHOLD_SCALE);

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_threshold(test_speed));
}

TEST_IN_TASK
void test_opd_thresh_8kmh(void) {
  float test_speed = 8.0f;
  float test_answer = (test_speed * COASTING_THRESHOLD_SCALE);

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_threshold(test_speed));
}

TEST_IN_TASK
void test_opd_thresh_16kmh(void) {
  float test_speed = 16.0f;
  float test_answer = (test_speed * COASTING_THRESHOLD_SCALE);

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_threshold(test_speed));
}

TEST_IN_TASK
void test_opd_thresh_20kmh(void) {
  float test_speed = 20.0f;
  float test_answer = MAX_COASTING_THRESHOLD;

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_threshold(test_speed));
}

TEST_IN_TASK
void test_opd_thresh_50kmh(void) {
  float test_speed = 50.0f;
  float test_answer = MAX_COASTING_THRESHOLD;

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_threshold(test_speed));
}

TEST_IN_TASK
void test_opd_thresh_0kmh(void) {
  float test_speed = 0.0f;
  float test_answer = 0.0f;

  /* At 0 speed the threshold is also 0 */
  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_threshold(test_speed));
}

TEST_IN_TASK
void test_opd_thresh_max_opd_speed(void) {
  float test_speed = MAX_OPD_SPEED;
  float test_answer = MAX_OPD_SPEED * COASTING_THRESHOLD_SCALE;

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_threshold(test_speed));
}

TEST_IN_TASK
void test_opd_thresh_just_above_max_opd_speed(void) {
  float test_speed = MAX_OPD_SPEED + 0.1f;
  float test_answer = MAX_COASTING_THRESHOLD;

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_threshold(test_speed));
}

TEST_IN_TASK
void test_opd_thresh_negative_speed(void) {
  float test_speed = -5.0f;
  float test_answer = 5.0f * COASTING_THRESHOLD_SCALE;

  /* Negative speed should be treated the same */
  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_threshold(test_speed));
}

TEST_IN_TASK
void test_opd_current_throttle_at_threshold(void) {
  float test_throttle_percent = 0.5f;
  float test_threshold = 0.5f;
  DriveState test_drive_state = VEHICLE_DRIVE;

  float test_answer = 0.0f;

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_current(test_throttle_percent, test_threshold, &test_drive_state));
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE, test_drive_state);
}

TEST_IN_TASK
void test_opd_current_throttle_above_threshold(void) {
  float test_throttle_percent = 0.7f;
  float test_threshold = 0.5f;
  DriveState test_drive_state = VEHICLE_DRIVE;

  float test_answer = (0.7f - 0.5f) / (1 - 0.5f);

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_current(test_throttle_percent, test_threshold, &test_drive_state));
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE, test_drive_state);
}

TEST_IN_TASK
void test_opd_current_throttle_at_zero(void) {
  float test_throttle_percent = 0.0f;
  float test_threshold = 0.5f;
  DriveState test_drive_state = VEHICLE_DRIVE;

  /* The output shall be 0 at a standstill */
  float test_answer = (0.5f - 0.0f) / 0.5f;  // Expected calculation

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_current(test_throttle_percent, test_threshold, &test_drive_state));
  TEST_ASSERT_EQUAL(VEHICLE_BRAKE, test_drive_state);
}

TEST_IN_TASK
void test_opd_current_throttle_at_full(void) {
  float test_throttle_percent = 1.0f;
  float test_threshold = 0.5f;
  DriveState test_drive_state = VEHICLE_DRIVE;

  /* The current output shall be maximum */
  float test_answer = 1.0f;

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_current(test_throttle_percent, test_threshold, &test_drive_state));
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE, test_drive_state);
}

TEST_IN_TASK
void test_opd_current_throttle_near_threshold(void) {
  float test_throttle_percent = 0.51f;
  float test_threshold = 0.5f;
  DriveState test_drive_state = VEHICLE_DRIVE;

  /* No acceleration, the user shall be coasting */
  float test_answer = 0.0f;

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_current(test_throttle_percent, test_threshold, &test_drive_state));
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE, test_drive_state);
}

TEST_IN_TASK
void test_opd_current_throttle_near_threshold_lower(void) {
  float test_throttle_percent = 0.49f;
  float test_threshold = 0.5f;
  DriveState test_drive_state = VEHICLE_DRIVE;

  /* No acceleration, the user shall be coasting */
  float test_answer = 0.0f;

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_current(test_throttle_percent, test_threshold, &test_drive_state));
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE, test_drive_state);
}

TEST_IN_TASK
void test_opd_current_throttle_at_threshold_with_brake_state(void) {
  float test_throttle_percent = 0.5f;
  float test_threshold = 0.5f;
  DriveState test_drive_state = VEHICLE_BRAKE;

  /* If braking and throttle percent is the same as the threshold it shall return 0 */
  float test_answer = 0.0f;

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_current(test_throttle_percent, test_threshold, &test_drive_state));
  TEST_ASSERT_EQUAL(VEHICLE_BRAKE, test_drive_state);
}

TEST_IN_TASK
void test_opd_current_throttle_negative_threshold(void) {
  float test_throttle_percent = 0.3f;
  float test_threshold = -0.5f; /* Negative threshold (invalid case) */
  DriveState test_drive_state = VEHICLE_DRIVE;

  /* Negative threshold is invalid, but the function should handle it gracefully by setting output to 0 */
  float test_answer = 0.0f;

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_current(test_throttle_percent, test_threshold, &test_drive_state));
  TEST_ASSERT_EQUAL(VEHICLE_BRAKE, test_drive_state); /* Should enter BRAKE mode */
}

TEST_IN_TASK
void test_opd_current_throttle_negative_throttle(void) {
  float test_throttle_percent = -0.2f; /* Negative throttle (invalid case) */
  float test_threshold = 0.5f;
  DriveState test_drive_state = VEHICLE_DRIVE;

  /* Negative throttle is invalid, but the function should handle it gracefully by setting output to 0 */
  float test_answer = 0.0f;

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_current(test_throttle_percent, test_threshold, &test_drive_state));
  TEST_ASSERT_EQUAL(VEHICLE_BRAKE, test_drive_state); /* Should enter BRAKE mode */
}

TEST_IN_TASK
void test_opd_current_throttle_below_threshold_with_regen(void) {
  float test_throttle_percent = 0.2f;
  float test_threshold = 0.5f;
  DriveState test_drive_state = VEHICLE_DRIVE;

  /* Throttle below threshold should trigger regen braking */
  float test_answer = (0.5f - 0.2f) / 0.5f;

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_current(test_throttle_percent, test_threshold, &test_drive_state));
  TEST_ASSERT_EQUAL(VEHICLE_BRAKE, test_drive_state); /* Should enter BRAKE mode */
}

TEST_IN_TASK
void test_opd_current_throttle_below_high_threshold_with_regen(void) {
  float test_throttle_percent = 0.6f;
  float test_threshold = 0.7f;
  DriveState test_drive_state = VEHICLE_DRIVE;

  /* Throttle below threshold should trigger regen braking */
  float test_answer = (0.7f - 0.6f) / 0.7f;

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_current(test_throttle_percent, test_threshold, &test_drive_state));
  TEST_ASSERT_EQUAL(VEHICLE_BRAKE, test_drive_state); /* Should enter BRAKE mode */
}

TEST_IN_TASK
void test_opd_current_throttle_below_low_threshold_with_regen(void) {
  float test_throttle_percent = 0.1f;
  float test_threshold = 0.3f;
  DriveState test_drive_state = VEHICLE_DRIVE;

  /* Throttle below threshold should trigger regen braking */
  float test_answer = (0.3f - 0.1f) / 0.3f;

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_current(test_throttle_percent, test_threshold, &test_drive_state));
  TEST_ASSERT_EQUAL(VEHICLE_BRAKE, test_drive_state); /* Should enter BRAKE mode */
}

TEST_IN_TASK
void test_opd_current_throttle_equal_max_threshold(void) {
  float test_throttle_percent = MAX_COASTING_THRESHOLD;
  float test_threshold = MAX_COASTING_THRESHOLD;
  DriveState test_drive_state = VEHICLE_DRIVE;

  /* There should be no output */
  float test_answer = 0.0f;

  TEST_ASSERT_FLOAT_WITHIN(0.001f, test_answer, opd_current(test_throttle_percent, test_threshold, &test_drive_state));
  TEST_ASSERT_EQUAL(VEHICLE_DRIVE, test_drive_state);
}
