/************************************************************************************************
 * @file   test_opd.c
 *
 * @brief  Unit tests for opd module
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
#include "front_controller_getters.h"
#include "front_controller_hw_defs.h"
#include "opd.h"

/**
 * NOTE: IS_OPD_ENABLED is currently 0 in opd.c. opd_run() does not perform the
 * OPD speed-matching algorithm — it only forwards accel/brake percentages to the CAN
 * TX struct. The public calculation API (opd_linear_calculate, opd_quadratic_calculate,
 * opd_calculate_handler) is still fully exercised below.
 *
 * opd_linear_calculate formula:
 *   current_speed = vehicle_speed_kph / max_vehicle_speed_kph
 *   if pedal > current_speed (PTS_TYPE_LINEAR):  driving
 *     result = 0.25 * (1/(1-speed)) * (pedal - 1) + 1
 *   else: braking
 *     result = max_braking * (1 - (1/speed) * pedal)
 *
 * opd_quadratic_calculate formula:
 *   if pedal^2 > current_speed (PTS_TYPE_QUADRATIC):  driving
 *     m = 1 / (1-speed)^2,  result = m * (pedal - speed)^2
 *   else: braking
 *     m = max_braking / speed^2,  result = m * (pedal - speed)^2
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
  mock_storage.brake_state = BRAKE_STATE_DISABLED;

  accel_pedal_init(&mock_storage);
  opd_init(&mock_storage);

  mock_storage.accel_pedal_storage->calibration_data.lower_value = 1000;
  mock_storage.accel_pedal_storage->calibration_data.upper_value = 2000;
  mock_storage.accel_pedal_storage->calibration_data.reversed = false;
  mock_storage.accel_pedal_storage->prev_accel_percentage = 0.0f;
  mock_storage.accel_pedal_storage->accel_percentage = 0.0f;

  mock_storage.opd_storage->max_vehicle_speed_kph = 100;
  mock_storage.opd_storage->max_braking_percentage = 0.75f;
}

void teardown_test(void) {}

/* ---- init tests ---- */

TEST_IN_TASK
void test_opd_init_null_storage_returns_error(void) {
  TEST_ASSERT_NOT_OK(opd_init(NULL));
}

/* ---- opd_run tests (IS_OPD_ENABLED = 0) ---- */

TEST_IN_TASK
void test_opd_run_without_brake_engaged_returns_ok(void) {
  mock_storage.brake_state = BRAKE_STATE_DISABLED;
  mock_storage.accel_percentage = 0.5f;
  mock_storage.brake_percentage = 0.0f;

  TEST_ASSERT_OK(opd_run());
}

TEST_IN_TASK
void test_opd_run_with_brake_engaged_returns_ok(void) {
  mock_storage.brake_state = BRAKE_STATE_BRAKING;
  mock_storage.accel_percentage = 0.0f;
  mock_storage.brake_percentage = 0.3f;

  TEST_ASSERT_OK(opd_run());
}

/* ---- opd_linear_calculate tests ---- */

TEST_IN_TASK
void test_opd_linear_calculate_braking(void) {
  /**
   * pedal=0.4, speed=50/100=0.5 → pedal < speed → braking
   * m = 1/speed = 1/0.5 = 2
   * result = max_braking * (1 - m*pedal) = 0.75 * (1 - 2*0.4) = 0.75 * 0.2 = 0.15
   */
  mock_storage.vehicle_speed_kph = 50;
  float result = 0.0f;
  TEST_ASSERT_OK(opd_linear_calculate(0.4f, PTS_TYPE_LINEAR, &result));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.15f, result);
}

TEST_IN_TASK
void test_opd_linear_calculate_driving(void) {
  /**
   * pedal=0.712, speed=70/100=0.7 → pedal > speed → driving
   * m = 1/(1-speed) = 1/0.3 ≈ 3.333
   * result = 0.25 * m * (pedal - 1) + 1 = 0.25*3.333*(-0.288) + 1 ≈ 0.76
   */
  mock_storage.vehicle_speed_kph = 70;
  float result = 0.0f;
  TEST_ASSERT_OK(opd_linear_calculate(0.712f, PTS_TYPE_LINEAR, &result));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.76f, result);
}

TEST_IN_TASK
void test_opd_linear_calculate_pedal_at_zero_full_braking(void) {
  /**
   * pedal=0.0, speed=0.6 → braking
   * m = 1/0.6 ≈ 1.667
   * result = 0.75 * (1 - 1.667*0) = 0.75
   */
  mock_storage.vehicle_speed_kph = 60;
  float result = 0.0f;
  TEST_ASSERT_OK(opd_linear_calculate(0.0f, PTS_TYPE_LINEAR, &result));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.75f, result);
}

/* ---- opd_quadratic_calculate tests ---- */

TEST_IN_TASK
void test_opd_quadratic_calculate_braking(void) {
  /**
   * pedal=0.4, speed=0.5 → PTS_TYPE_QUADRATIC: pedal^2=0.16 < speed=0.5 → braking
   * m = max_braking / speed^2 = 0.75 / 0.25 = 3
   * result = m * (pedal - speed)^2 = 3 * (0.4 - 0.5)^2 = 3 * 0.01 = 0.03
   */
  mock_storage.vehicle_speed_kph = 50;
  float result = 0.0f;
  TEST_ASSERT_OK(opd_quadratic_calculate(0.4f, PTS_TYPE_QUADRATIC, &result));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.03f, result);
}

TEST_IN_TASK
void test_opd_quadratic_calculate_driving(void) {
  /**
   * pedal=0.9, speed=0.5 → PTS_TYPE_QUADRATIC: pedal^2=0.81 > speed=0.5 → driving
   * m = 1 / (1-speed)^2 = 1 / 0.25 = 4
   * result = m * (pedal - speed)^2 = 4 * (0.9 - 0.5)^2 = 4 * 0.16 = 0.64
   */
  mock_storage.vehicle_speed_kph = 50;
  float result = 0.0f;
  TEST_ASSERT_OK(opd_quadratic_calculate(0.9f, PTS_TYPE_QUADRATIC, &result));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.64f, result);
}

/* ---- opd_calculate_handler curve type routing tests ---- */

TEST_IN_TASK
void test_opd_calculate_handler_routes_linear_curve(void) {
  /* Same inputs as test_opd_linear_calculate_braking — confirms routing */
  mock_storage.vehicle_speed_kph = 50;
  float result = 0.0f;
  TEST_ASSERT_OK(opd_calculate_handler(0.4f, PTS_TYPE_LINEAR, &result, CURVE_TYPE_LINEAR));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.15f, result);
}

TEST_IN_TASK
void test_opd_calculate_handler_routes_quadratic_curve(void) {
  /* Same inputs as test_opd_quadratic_calculate_braking — confirms routing */
  mock_storage.vehicle_speed_kph = 50;
  float result = 0.0f;
  TEST_ASSERT_OK(opd_calculate_handler(0.4f, PTS_TYPE_QUADRATIC, &result, CURVE_TYPE_QUADRATIC));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.03f, result);
}

TEST_IN_TASK
void test_opd_calculate_handler_exponential_applies_exponent_to_linear(void) {
  /**
   * CURVE_TYPE_EXPONENTIAL runs opd_linear_calculate then raises to config exponent (2.0).
   * pedal=0.4, speed=0.5 → linear braking result = 0.15
   * exponential: 0.15^2 = 0.0225
   */
  mock_storage.vehicle_speed_kph = 50;
  float result = 0.0f;
  TEST_ASSERT_OK(opd_calculate_handler(0.4f, PTS_TYPE_LINEAR, &result, CURVE_TYPE_EXPONENTIAL));
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0225f, result);
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

  fprintf(f, "pedal_percentage,speed_percentage,linear_result,quadratic_result\n");

  for (uint32_t speed_kph = 0; speed_kph <= 100; speed_kph += 10) {
    mock_storage.vehicle_speed_kph = speed_kph;

    for (int pedal_pct = 0; pedal_pct <= 100; pedal_pct += 5) {
      float pedal = (float)pedal_pct / 100.0f;
      float speed_norm = (float)speed_kph / 100.0f;

      float linear_result = 0.0f;
      float quadratic_result = 0.0f;

      if (speed_kph > 0) {
        opd_linear_calculate(pedal, PTS_TYPE_LINEAR, &linear_result);
        opd_quadratic_calculate(pedal, PTS_TYPE_QUADRATIC, &quadratic_result);
      }

      fprintf(f, "%.2f,%.2f,%.5f,%.5f\n", (double)pedal, (double)speed_norm, (double)linear_result, (double)quadratic_result);
    }
  }

  fclose(f);
  TEST_PASS_MESSAGE("Generated opd_curve.csv in test_results/");
}
