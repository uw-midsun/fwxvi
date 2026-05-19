/************************************************************************************************
 * @file   test_accel_pedal.c
 *
 * @brief  Unit tests for accel_pedal module
 *
 * @date   2026-05-15
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <libgen.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Inter-component Headers */
#include "adc.h"
#include "gpio.h"
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "accel_pedal.h"
#include "front_controller.h"
#include "front_controller_hw_defs.h"

/**
 * Algorithm implemented in accel_pedal.c:
 *   1. normalized = (adc - lower) / (upper - lower), clamped to [0, 1]
 *   2. if normalized < deadzone → 0; else → powf(normalized, exponent)
 *      then remap: remap_min + (1 - remap_min) * shaped
 *   3. if calibration_data.reversed: value = 1.0f - value
 *   4. value = value^2 - 0.08, clamped to [0, ∞)
 *   5. LPF: alpha * value + (1 - alpha) * prev_accel_percentage
 *      NOTE: prev_accel_percentage stores the pre-LPF input (not the filtered output)
 *   6. if accel_percentage <= 0.05 → clamp to 0
 */

static FrontControllerStorage mock_storage = { 0 };
static uint16_t mock_raw_adc_reading = 0;

StatusCode TEST_MOCK(adc_read_raw)(GpioAddress *addr, uint16_t *reading) {
  if (reading == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  *reading = mock_raw_adc_reading;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  static FrontControllerConfig config = {
    .accel_input_deadzone = 0.05f,
    .accel_input_curve_exponent = 2.0f,
    .accel_input_remap_min = 0.2f,
    .accel_low_pass_filter_alpha = 1.0f,
  };

  mock_storage.config = &config;

  accel_pedal_init(&mock_storage);

  mock_storage.accel_pedal_storage->calibration_data.lower_value = 1000;
  mock_storage.accel_pedal_storage->calibration_data.upper_value = 2000;
  mock_storage.accel_pedal_storage->calibration_data.reversed = false;
  mock_storage.accel_pedal_storage->prev_accel_percentage = 0.0f;
  mock_storage.accel_pedal_storage->accel_percentage = 0.0f;
}

void teardown_test(void) {}

TEST_IN_TASK
void test_accel_pedal_init_null_storage_returns_error(void) {
  TEST_ASSERT_NOT_OK(accel_pedal_init(NULL));
}

TEST_IN_TASK
void test_accel_pedal_midscale_normalized_reading(void) {
  /**
   * normalized = (1500 - 1000) / (2000 - 1000) = 0.5
   * above deadzone (0.05)
   * powf(0.5, 2.0) = 0.25
   * remap: 0.2 + 0.8 * 0.25 = 0.4
   * not reversed
   * squared: 0.4^2 = 0.16, offset: 0.16 - 0.08 = 0.08
   * LPF alpha=1.0: 0.08
   * 0.08 > 0.05 threshold → 0.08
   */
  mock_raw_adc_reading = 1500;
  TEST_ASSERT_OK(accel_pedal_run());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.08f, mock_storage.accel_pedal_storage->accel_percentage);
}

TEST_IN_TASK
void test_accel_pedal_below_deadzone_returns_zero(void) {
  /* normalized = (1040 - 1000) / 1000 = 0.04 < deadzone 0.05 → 0 */
  mock_raw_adc_reading = 1040;
  TEST_ASSERT_OK(accel_pedal_run());
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, mock_storage.accel_pedal_storage->accel_percentage);
}

TEST_IN_TASK
void test_accel_pedal_full_scale_returns_correct_percentage(void) {
  /**
   * normalized = 1.0
   * powf(1.0, 2.0) = 1.0
   * remap: 0.2 + 0.8 * 1.0 = 1.0
   * squared: 1.0^2 = 1.0, offset: 1.0 - 0.08 = 0.92
   */
  mock_raw_adc_reading = 2000;
  TEST_ASSERT_OK(accel_pedal_run());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.92f, mock_storage.accel_pedal_storage->accel_percentage);
}

TEST_IN_TASK
void test_accel_pedal_above_calibration_range_clamped_to_full_scale(void) {
  /* normalized = (2500 - 1000) / 1000 = 1.5, clamped to 1.0 → same as full scale */
  mock_raw_adc_reading = 2500;
  TEST_ASSERT_OK(accel_pedal_run());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.92f, mock_storage.accel_pedal_storage->accel_percentage);
}

TEST_IN_TASK
void test_accel_pedal_below_calibration_range_clamped_to_zero(void) {
  /* normalized = (500 - 1000) / 1000 = -0.5, clamped to 0.0 → below deadzone → 0 */
  mock_raw_adc_reading = 500;
  TEST_ASSERT_OK(accel_pedal_run());
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, mock_storage.accel_pedal_storage->accel_percentage);
}

TEST_IN_TASK
void test_accel_pedal_reversed_pedal_flips_output(void) {
  /**
   * normalized = 0.5, above deadzone
   * powf(0.5, 2.0) = 0.25, remap = 0.4
   * reversed=true: 1.0 - 0.4 = 0.6
   * squared: 0.6^2 = 0.36, offset: 0.36 - 0.08 = 0.28
   */
  mock_storage.accel_pedal_storage->calibration_data.reversed = true;
  mock_raw_adc_reading = 1500;
  TEST_ASSERT_OK(accel_pedal_run());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.28f, mock_storage.accel_pedal_storage->accel_percentage);
}

TEST_IN_TASK
void test_accel_pedal_lpf_alpha_less_than_one_smooths_output(void) {
  /**
   * prev_accel_percentage stores the pre-LPF input (not the filtered output),
   * so after just one transition the filter output converges to the new input
   * on the very next call.
   *
   * Call 1 (adc=2000, prev=0): LPF = 0.5*0.92 + 0.5*0 = 0.46, prev becomes 0.92
   * Call 2 (adc=2000, prev=0.92): LPF = 0.5*0.92 + 0.5*0.92 = 0.92
   */
  mock_storage.config->accel_low_pass_filter_alpha = 0.5f;
  mock_storage.accel_pedal_storage->prev_accel_percentage = 0.0f;

  mock_raw_adc_reading = 2000;
  TEST_ASSERT_OK(accel_pedal_run());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.46f, mock_storage.accel_pedal_storage->accel_percentage);

  TEST_ASSERT_OK(accel_pedal_run());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.92f, mock_storage.accel_pedal_storage->accel_percentage);
}

TEST_IN_TASK
void test_accel_pedal_result_below_threshold_clamped_to_zero(void) {
  /**
   * A reading that produces a pre-clamp result <= 0.05 is forced to 0.
   *
   * adc=1100: normalized=0.1, powf(0.1, 2)=0.01, remap=0.208
   * squared: 0.208^2=0.0433, offset: 0.0433 - 0.08 = -0.0367 → clamped to 0
   */
  mock_raw_adc_reading = 1100;
  TEST_ASSERT_OK(accel_pedal_run());
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, mock_storage.accel_pedal_storage->accel_percentage);
}

void test_accel_pedal_visualize_curve(void) {
  char out_dir[512];
  char out_file[512];
  char file_path[512];

  strncpy(file_path, __FILE__, sizeof(file_path) - 1);
  file_path[sizeof(file_path) - 1] = '\0';

  char *dir = dirname(file_path);

  int remaining_space = sizeof(out_dir) - strlen(dir) - 1;
  if (remaining_space <= 0) {
    TEST_FAIL_MESSAGE("Directory path is too long for buffer.");
    return;
  }

  snprintf(out_dir, remaining_space, "%s/test_results", dir);
  snprintf(out_file, sizeof(out_file), "%s/accel_pedal_curve.csv", out_dir);

  struct stat st = { 0 };
  if (stat(out_dir, &st) == -1) {
    mkdir(out_dir, 0755);
  }

  FILE *f = fopen(out_file, "w");
  TEST_ASSERT_NOT_NULL(f);

  fprintf(f, "adc_input,accel_percentage\n");

  mock_storage.accel_pedal_storage->calibration_data.reversed = false;
  mock_storage.config->accel_low_pass_filter_alpha = 1.0f;

  for (uint16_t adc_val = mock_storage.accel_pedal_storage->calibration_data.lower_value;
       adc_val <= mock_storage.accel_pedal_storage->calibration_data.upper_value; adc_val += 10) {
    mock_raw_adc_reading = adc_val;
    mock_storage.accel_pedal_storage->prev_accel_percentage = 0.0f;
    accel_pedal_run();
    fprintf(f, "%u,%.5f\n", adc_val, (double)mock_storage.accel_pedal_storage->accel_percentage);
  }

  fclose(f);
  TEST_PASS_MESSAGE("Generated accel_pedal_curve.csv in test_results/");
}
