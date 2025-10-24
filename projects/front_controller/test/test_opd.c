/************************************************************************************************
 * @file   test_opd.c
 *
 * @brief  Unit tests for opd module
 *
 * @date   2025-10-03
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
#include "opd.h"

static FrontControllerStorage mock_storage = { 0 };
static GpioAddress mock_accel_pedal_gpio = FRONT_CONTROLLER_ACCEL_PEDAL;
static uint16_t mock_raw_adc_reading = 0;

StatusCode TEST_MOCK(adc_read_raw)(GpioAddress *addr, uint16_t *reading) {
  if (reading == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // float normalized_mock_accel_percentage = (((float)mock_raw_adc_reading - (float)mock_storage.accel_pedal_storage->calibration_data.lower_value) /
  //                                           ((float)mock_storage.accel_pedal_storage->calibration_data.upper_value - (float)mock_storage.accel_pedal_storage->calibration_data.lower_value));

  // normalized_mock_accel_percentage = fminf(fmaxf(normalized_mock_accel_percentage, 0.0f), 1.0f);

  // *reading = normalized_mock_accel_percentage;
  *reading = mock_raw_adc_reading;

  return STATUS_CODE_OK;
}

void setup_test(void) {
  static FrontControllerConfig config = {
    .accel_input_deadzone = 0.1f,
    .accel_input_curve_exponent = 2.0f,
    .accel_input_remap_min = 0.2f,
    .accel_low_pass_filter_alpha = 1.0f,
  };

  mock_storage.config = &config;

  opd_init(&mock_storage);

  mock_storage.opd_storage->calibration_data.lower_value = 1000;
  mock_storage.opd_storage->calibration_data.upper_value = 2000;
  mock_storage.opd_storage->prev_accel_percentage = 0.0f;
  mock_storage.opd_storage->accel_percentage = 0.0f;
}

void teardown_test(void) {}

void test_opd_visualize_curve(void) {
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
  snprintf(out_file, sizeof(out_file), "%s/opd_curve.csv", out_dir);

  struct stat st = { 0 };
  if (stat(out_dir, &st) == -1) {
    mkdir(out_dir, 0755);
  }

  FILE *f = fopen(out_file, "w");
  TEST_ASSERT_NOT_NULL(f);

  fprintf(f, "adc_input,adc_input_normalized,accel_percentage,accel_state,current_speed\n");

  for (uint16_t adc_val = mock_storage.opd_storage->calibration_data.lower_value; adc_val <= mock_storage.opd_storage->calibration_data.upper_value; adc_val += 10) {
    for (uint32_t speed = 0; speed <= 100; speed += 1) {
      mock_raw_adc_reading = adc_val;

      float normalized_accel_percentage = (((float)adc_val - (float)mock_storage.opd_storage->calibration_data.lower_value) /
                                           ((float)mock_storage.opd_storage->calibration_data.upper_value - (float)mock_storage.opd_storage->calibration_data.lower_value));

      normalized_accel_percentage = fminf(fmaxf(normalized_accel_percentage, 0.0f), 1.0f);

      float calculated_reading;

      mock_storage.vehicle_speed_kph = speed;
      mock_storage.opd_storage->max_vehicle_speed_kph = 100;
      mock_storage.opd_storage->max_braking_percentage = 0.75;
      mock_storage.config->accel_input_curve_exponent = 2;

      float normalized_speed = ((float)mock_storage.vehicle_speed_kph / (float)mock_storage.opd_storage->max_vehicle_speed_kph);

      opd_calculate_handler(normalized_accel_percentage, PTS_TYPE_LINEAR, &calculated_reading, CURVE_TYPE_EXPONENTIAL);

      fprintf(f, "%u,%.5f,%.2f,%u,%.5f\n", adc_val, (double)normalized_accel_percentage, (double)calculated_reading, mock_storage.opd_storage->accel_state, (double)normalized_speed);
    }
  }

  fclose(f);
  TEST_PASS_MESSAGE("Generated opd_curve.csv in test_results/");
}
