/************************************************************************************************
 * @file   test_brake_pedal.c
 *
 * @brief  Unit tests for brake_pedal module
 *
 * @date   2025-09-15
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
#include "brake_pedal.h"
#include "front_controller.h"
#include "front_controller_hw_defs.h"

static FrontControllerStorage mock_storage = { 0 };
static GpioAddress mock_brake_pedal_gpio = FRONT_CONTROLLER_BRAKE_PEDAL;
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
    .brake_pedal_deadzone = 0.05f,
    .brake_low_pass_filter_alpha = 1.0f,
  };

  mock_storage.config = &config;

  brake_pedal_init(&mock_storage);

  mock_storage.brake_pedal_storage->calibration_data.lower_value = 1000;
  mock_storage.brake_pedal_storage->calibration_data.upper_value = 2000;
  mock_storage.brake_pedal_storage->prev_reading = 0.0f;
  mock_storage.brake_enabled = false;
}

void teardown_test(void) {}

TEST_IN_TASK
void test_brake_pedal_reading_below_deadzone_should_disable_brake(void) {
  /* Normalized = 0.04 */
  mock_raw_adc_reading = 1040;
  StatusCode ret = brake_pedal_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  TEST_ASSERT_FALSE(mock_storage.brake_enabled);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.04f, mock_storage.brake_pedal_storage->prev_reading);
}

TEST_IN_TASK
void test_brake_pedal_reading_above_deadzone_should_enable_brake(void) {
  /* Normalized = 0.1 */
  mock_raw_adc_reading = 1100;
  StatusCode ret = brake_pedal_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  TEST_ASSERT_TRUE(mock_storage.brake_enabled);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.1f, mock_storage.brake_pedal_storage->prev_reading);
}

TEST_IN_TASK
void test_brake_pedal_low_pass_filtering_effect(void) {
  /* Normalized = 0.2 */
  mock_raw_adc_reading = 1200;
  mock_storage.config->brake_low_pass_filter_alpha = 0.5f;
  mock_storage.brake_pedal_storage->prev_reading = 0.0f;

  StatusCode ret = brake_pedal_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  float expected = 0.5f * 0.2f + 0.5f * 0.0f;
  TEST_ASSERT_FLOAT_WITHIN(0.001f, expected, mock_storage.brake_pedal_storage->prev_reading);

  ret = brake_pedal_run();
  expected = 0.5f * 0.2f + 0.5f * 0.1f;
  TEST_ASSERT_FLOAT_WITHIN(0.001f, expected, mock_storage.brake_pedal_storage->prev_reading);
}

TEST_IN_TASK
void test_brake_pedal_fully_pressed(void) {
  /* Normalized = 1.0 */
  mock_raw_adc_reading = 2000;
  mock_storage.config->brake_low_pass_filter_alpha = 1.0f;
  mock_storage.brake_pedal_storage->prev_reading = 0.0f;
  StatusCode ret = brake_pedal_run();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  TEST_ASSERT_TRUE(mock_storage.brake_enabled);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, mock_storage.brake_pedal_storage->prev_reading);
}
