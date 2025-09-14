/************************************************************************************************
 * @file   test_accel_pedal.c
 *
 * @brief  Unit tests for accel_pedal module
 *
 * @date   2025-09-10
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

static FrontControllerStorage mock_storage = { 0 };
static GpioAddress mock_accel_pedal_gpio = FRONT_CONTROLLER_ACCEL_PEDAL;
static uint16_t mock_raw_adc_reading = 0;

StatusCode TEST_MOCK(adc_read_aw)(GpioAddress *addr, uint16_t *reading) {
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
  mock_storage.accel_pedal_storage->prev_accel_percentage = 0.0f;
  mock_storage.accel_pedal_storage->accel_percentage = 0.0f;
}

void teardown_test(void) {}

TEST_IN_TASK
void test_accel_pedal_normalization_and_clamping(void) {
  mock_raw_adc_reading = 1500;
  StatusCode ret = accel_pedal_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  /**
   * Normalized = (1500-1000)/(2000-1000) = 0.5
   * Deadzone = 0.05 (ignored)
   * pow(0.5, 2.0) = 0.25
   * Remap_min=0.2, remapped = 0.2 + (1-0.2)*0.25 = 0.4
   */
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.4f, mock_storage.accel_pedal_storage->accel_percentage);
}

TEST_IN_TASK
void test_accel_pedal_deadzone_zero(void) {
  /* Normalized = (1040-1000)/1000 = 0.04 < deadzone 0.05 */
  mock_raw_adc_reading = 1040;
  StatusCode ret = accel_pedal_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, mock_storage.accel_pedal_storage->accel_percentage);
}

TEST_IN_TASK
void test_accel_pedal_full_scale(void) {
  /* Normalized = 1.0 */
  mock_raw_adc_reading = 2000;
  StatusCode ret = accel_pedal_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  /**
   * pow(1, exponent) = 1
   * remap = 0.2 + (1-0.2)*1 = 1.0
   */
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, mock_storage.accel_pedal_storage->accel_percentage);
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

  fprintf(f, "adc_input,normalized,after_deadzone,after_curve,remapped_output\n");

  for (uint16_t adc_val = mock_storage.accel_pedal_storage->calibration_data.lower_value; adc_val <= mock_storage.accel_pedal_storage->calibration_data.upper_value; adc_val += 10) {
    float normalized = ((float)adc_val - (float)mock_storage.accel_pedal_storage->calibration_data.lower_value) /
                       ((float)mock_storage.accel_pedal_storage->calibration_data.upper_value - (float)mock_storage.accel_pedal_storage->calibration_data.lower_value);

    if (normalized < 0.0f) normalized = 0.0f;
    if (normalized > 1.0f) normalized = 1.0f;

    float after_deadzone = normalized;
    if (after_deadzone < mock_storage.config->accel_input_deadzone) {
      after_deadzone = 0.0f;
    }

    float after_curve = powf(after_deadzone, mock_storage.config->accel_input_curve_exponent);

    float remapped = mock_storage.config->accel_input_remap_min + ((1.0f - mock_storage.config->accel_input_remap_min) * after_curve);

    fprintf(f, "%u,%.5f,%.5f,%.5f,%.5f\n", adc_val, normalized, after_deadzone, after_curve, remapped);
  }

  fclose(f);
  TEST_PASS_MESSAGE("Generated accel_pedal_curve.csv in test_results/");
}
