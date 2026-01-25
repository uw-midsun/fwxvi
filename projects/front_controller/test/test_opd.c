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
#include "front_controller_getters.h"
#include "front_controller_hw_defs.h"
#include "opd.h"

static FrontControllerStorage mock_storage = { 0 };
static GpioAddress mock_accel_pedal_gpio = FRONT_CONTROLLER_ACCEL_PEDAL;
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
  opd_init(&mock_storage);

  mock_storage.accel_pedal_storage->calibration_data.lower_value = 1000;
  mock_storage.accel_pedal_storage->calibration_data.upper_value = 2000;
  mock_storage.accel_pedal_storage->prev_accel_percentage = 0.0f;
  mock_storage.accel_pedal_storage->accel_percentage = 0.0f;
  mock_storage.opd_storage->max_vehicle_speed_kph = 100;
  mock_storage.opd_storage->max_braking_percentage = 0.75;
  // Disable regen limiter by placing voltage under 4150mV to avoid interfering with other tests
  g_rx_struct.battery_stats_B_min_cell_voltage = 4000;
}

void teardown_test(void) {}

TEST_IN_TASK
void test_opd_braking(void) {
  mock_raw_adc_reading = 1500;
  mock_storage.vehicle_speed_kph = 50;
  StatusCode ret = accel_pedal_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  ret = opd_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  /**
   * Normalized = (1500-1000)/(2000-1000) = 0.5
   * Deadzone = 0.05 (ignored)
   * pow(0.5, 2.0) = 0.25
   * Remap_min=0.2, remapped = 0.2 + (1-0.2)*0.25 = 0.4
   *
   * Current speed = 50 / 100 = 0.5
   * Since 0.4 < 5, it should be braking
   * 0.75 * (1 - (0.4 / 0.5)) = 0.15
   *
   */
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.4f, mock_storage.accel_pedal_storage->accel_percentage);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.15f, mock_storage.accel_percentage);
  TEST_ASSERT_EQUAL(1, mock_storage.brake_enabled);
  TEST_ASSERT_EQUAL(STATE_BRAKING, mock_storage.opd_storage->drive_state);
}

TEST_IN_TASK
void test_opd_regen_no_limiting(void) {
  // Simulate braking with no regen limiting
  mock_raw_adc_reading = 1500;
  mock_storage.vehicle_speed_kph = 50;
  g_rx_struct.battery_stats_B_min_cell_voltage = 4100;  // Below the range of 4.15 to 4.2V

  StatusCode ret = accel_pedal_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  ret = opd_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  /**
   * Normalized = (1500-1000)/(2000-1000) = 0.5
   * Deadzone = 0.05 (ignored)
   * pow(0.5, 2.0) = 0.25
   * Remap_min=0.2, remapped = 0.2 + (1-0.2)*0.25 = 0.4
   *
   * Current speed = 50 / 100 = 0.5
   * Since 0.4 < 5, it should be braking
   * 0.75 * (1 - (0.4 / 0.5)) = 0.15
   *
   */
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.4f, mock_storage.accel_pedal_storage->accel_percentage);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.15f, mock_storage.accel_percentage);
  TEST_ASSERT_EQUAL(1, mock_storage.brake_enabled);
  TEST_ASSERT_EQUAL(STATE_BRAKING, mock_storage.opd_storage->drive_state);
}

TEST_IN_TASK
void test_opd_regen_partial_limiting(void) {
  // Simulate braking with cell voltage that should require partial
  // regen limiting (between 4.15 to 4.2V)
  mock_raw_adc_reading = 1500;
  mock_storage.vehicle_speed_kph = 50;
  g_rx_struct.battery_stats_B_min_cell_voltage = 4175;

  StatusCode ret = accel_pedal_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  ret = opd_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  /**
   * Normalized = (1500-1000)/(2000-1000) = 0.5
   * Deadzone = 0.05 (ignored)
   * pow(0.5, 2.0) = 0.25
   * Remap_min=0.2, remapped = 0.2 + (1-0.2)*0.25 = 0.4
   *
   * Current speed = 50 / 100 = 0.5
   * Since 0.4 < 5, it should be braking
   * 0.75 * (1 - (0.4 / 0.5)) = 0.15
   * Since cell voltage (in mV) in range [4150, 4200], should be half of initial value
   * 0.15 * 0.5 = 0.075
   *
   */
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.4f, mock_storage.accel_pedal_storage->accel_percentage);
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.075f, mock_storage.accel_percentage);
  TEST_ASSERT_EQUAL(1, mock_storage.brake_enabled);
  TEST_ASSERT_EQUAL(STATE_BRAKING, mock_storage.opd_storage->drive_state);
}

TEST_IN_TASK
void test_opd_regen_full_limiting(void) {
  // Simulate braking with cell voltage that should completely limit regen (max cell voltage)
  mock_raw_adc_reading = 1500;
  mock_storage.vehicle_speed_kph = 50;
  g_rx_struct.battery_stats_B_min_cell_voltage = 4200;

  StatusCode ret = accel_pedal_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  ret = opd_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  /**
   * Normalized = (1500-1000)/(2000-1000) = 0.5
   * Deadzone = 0.05 (ignored)
   * pow(0.5, 2.0) = 0.25
   * Remap_min=0.2, remapped = 0.2 + (1-0.2)*0.25 = 0.4
   *
   * Current speed = 50 / 100 = 0.5
   * Since 0.4 < 5, it should be braking
   * 0.75 * (1 - (0.4 / 0.5)) = 0.15
   * Since cell voltage is at max, should be 0
   *
   */
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.4f, mock_storage.accel_pedal_storage->accel_percentage);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, mock_storage.accel_percentage);
  TEST_ASSERT_EQUAL(1, mock_storage.brake_enabled);
  TEST_ASSERT_EQUAL(STATE_BRAKING, mock_storage.opd_storage->drive_state);
}

TEST_IN_TASK
void test_opd_regen_no_expected_limiting(void) {
  // Simulate accelerating with cell voltage (mV) in [4150, 4200]
  mock_raw_adc_reading = 1800;
  mock_storage.vehicle_speed_kph = 70;
  g_rx_struct.battery_stats_B_min_cell_voltage = 4175;

  StatusCode ret = accel_pedal_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  ret = opd_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  /**
   * Normalized = (1500-1000)/(2000-1000) = 0.8
   * Deadzone = 0.05 (ignored)
   * pow(0.8, 2.0) = 0.64
   * Remap_min=0.2, remapped = 0.2 + (1-0.2)*0.64 = 0.712
   *
   * Current speed = 70 / 100 = 0.7
   * Since 0.712 > 0.7, it should be driving
   * 0.25 * (1 / (1 - 0.7)) * (0.712 - 1) + 1 = 0.76
   * This shouldn't be touched by the regen limiter since it's driving
   *
   */
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.712f, mock_storage.accel_pedal_storage->accel_percentage);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.76f, mock_storage.accel_percentage);
  TEST_ASSERT_EQUAL(0, mock_storage.brake_enabled);
  TEST_ASSERT_EQUAL(STATE_DRIVING, mock_storage.opd_storage->drive_state);

  // Simulate accelerating with max cell voltage
  g_rx_struct.battery_stats_B_min_cell_voltage = 4200;

  ret = accel_pedal_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  ret = opd_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  /**
   * Normalized = (1500-1000)/(2000-1000) = 0.8
   * Deadzone = 0.05 (ignored)
   * pow(0.8, 2.0) = 0.64
   * Remap_min=0.2, remapped = 0.2 + (1-0.2)*0.64 = 0.712
   *
   * Current speed = 70 / 100 = 0.7
   * Since 0.712 > 0.7, it should be driving
   * 0.25 * (1 / (1 - 0.7)) * (0.712 - 1) + 1 = 0.76
   * This shouldn't be touched by the regen limiter since it's driving
   *
   */
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.712f, mock_storage.accel_pedal_storage->accel_percentage);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.76f, mock_storage.accel_percentage);
  TEST_ASSERT_EQUAL(0, mock_storage.brake_enabled);
  TEST_ASSERT_EQUAL(STATE_DRIVING, mock_storage.opd_storage->drive_state);
}

TEST_IN_TASK
void test_opd_driving(void) {
  mock_raw_adc_reading = 1800;
  mock_storage.vehicle_speed_kph = 70;
  StatusCode ret = accel_pedal_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  ret = opd_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  /**
   * Normalized = (1500-1000)/(2000-1000) = 0.8
   * Deadzone = 0.05 (ignored)
   * pow(0.8, 2.0) = 0.64
   * Remap_min=0.2, remapped = 0.2 + (1-0.2)*0.64 = 0.712
   *
   * Current speed = 70 / 100 = 0.7
   * Since 0.712 > 0.7, it should be driving
   * 0.25 * (1 / (1 - 0.7)) * (0.712 - 1) + 1 = 0.76
   *
   */
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.712f, mock_storage.accel_pedal_storage->accel_percentage);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.76f, mock_storage.accel_percentage);
  TEST_ASSERT_EQUAL(0, mock_storage.brake_enabled);
  TEST_ASSERT_EQUAL(STATE_DRIVING, mock_storage.opd_storage->drive_state);
}

TEST_IN_TASK
void test_opd_deadzone(void) {
  /* Normalized = (1040-1000)/1000 = 0.04 < deadzone 0.05 */
  mock_raw_adc_reading = 1040;
  mock_storage.vehicle_speed_kph = 60;
  StatusCode ret = accel_pedal_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  ret = opd_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  /**
   * Current speed = 70 / 100 = 0.6
   * Since 0.0 < 0.7, it should be braking
   * 0.75 * (1 - (1/0.6 * 0.0)) = 0.75
   */
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, mock_storage.accel_pedal_storage->accel_percentage);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.75f, mock_storage.accel_percentage);
  TEST_ASSERT_EQUAL(1, mock_storage.brake_enabled);
  TEST_ASSERT_EQUAL(STATE_BRAKING, mock_storage.opd_storage->drive_state);
}

TEST_IN_TASK
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

  for (uint16_t adc_val = mock_storage.accel_pedal_storage->calibration_data.lower_value; adc_val <= mock_storage.accel_pedal_storage->calibration_data.upper_value; adc_val += 10) {
    for (uint32_t speed = 0; speed <= 100; speed += 1) {
      mock_raw_adc_reading = adc_val;
      mock_storage.vehicle_speed_kph = speed;
      float normalized_speed = ((float)mock_storage.vehicle_speed_kph / (float)mock_storage.opd_storage->max_vehicle_speed_kph);

      accel_pedal_run();
      opd_run();

      fprintf(f, "%u,%.5f,%.2f,%u,%.5f\n", adc_val, mock_storage.accel_pedal_storage->accel_percentage, mock_storage.accel_percentage, mock_storage.opd_storage->drive_state, (double)normalized_speed);
    }
  }

  fclose(f);
  TEST_PASS_MESSAGE("Generated opd_curve.csv in test_results/");
}
