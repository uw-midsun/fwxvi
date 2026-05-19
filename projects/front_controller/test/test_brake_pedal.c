/************************************************************************************************
 * @file   test_brake_pedal.c
 *
 * @brief  Unit tests for brake_pedal module
 *
 * @date   2026-05-15
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "adc.h"
#include "gpio.h"
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "brake_pedal.h"
#include "front_controller.h"
#include "front_controller_hw_defs.h"

/**
 * Algorithm implemented in brake_pedal.c:
 *   1. normalized = (adc - lower) / (upper - lower)   (no clamping)
 *   2. LPF: alpha * normalized + (1 - alpha) * prev_reading
 *      prev_reading stores the LPF output (standard IIR)
 *   3. if filtered > deadzone → BRAKE_STATE_BRAKING; else BRAKE_STATE_DISABLED
 *   4. brake_percentage = filtered reading
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
    .brake_pedal_deadzone = 0.05f,
    .brake_low_pass_filter_alpha = 1.0f,
  };

  mock_storage.config = &config;
  mock_storage.brake_state = BRAKE_STATE_DISABLED;

  brake_pedal_init(&mock_storage);

  mock_storage.brake_pedal_storage->calibration_data.lower_value = 1000;
  mock_storage.brake_pedal_storage->calibration_data.upper_value = 2000;
  mock_storage.brake_pedal_storage->prev_reading = 0.0f;
}

void teardown_test(void) {}

TEST_IN_TASK
void test_brake_pedal_init_null_storage_returns_error(void) {
  TEST_ASSERT_NOT_OK(brake_pedal_init(NULL));
}

TEST_IN_TASK
void test_brake_pedal_reading_below_deadzone_disables_brake(void) {
  /* normalized = (1040 - 1000) / 1000 = 0.04, LPF alpha=1.0 → 0.04 < deadzone 0.05 */
  mock_raw_adc_reading = 1040;
  TEST_ASSERT_OK(brake_pedal_run());

  TEST_ASSERT_EQUAL(BRAKE_STATE_DISABLED, mock_storage.brake_state);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.04f, mock_storage.brake_pedal_storage->prev_reading);
}

TEST_IN_TASK
void test_brake_pedal_reading_above_deadzone_enables_brake(void) {
  /* normalized = (1100 - 1000) / 1000 = 0.1, LPF alpha=1.0 → 0.1 > deadzone 0.05 */
  mock_raw_adc_reading = 1100;
  TEST_ASSERT_OK(brake_pedal_run());

  TEST_ASSERT_EQUAL(BRAKE_STATE_BRAKING, mock_storage.brake_state);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.1f, mock_storage.brake_pedal_storage->prev_reading);
}

TEST_IN_TASK
void test_brake_pedal_low_pass_filtering_effect(void) {
  /**
   * normalized = (1200 - 1000) / 1000 = 0.2, alpha=0.5
   * Call 1: LPF = 0.5*0.2 + 0.5*0.0 = 0.1,   prev becomes 0.1
   * Call 2: LPF = 0.5*0.2 + 0.5*0.1 = 0.15,  prev becomes 0.15
   */
  mock_raw_adc_reading = 1200;
  mock_storage.config->brake_low_pass_filter_alpha = 0.5f;
  mock_storage.brake_pedal_storage->prev_reading = 0.0f;

  TEST_ASSERT_OK(brake_pedal_run());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.1f, mock_storage.brake_pedal_storage->prev_reading);

  TEST_ASSERT_OK(brake_pedal_run());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.15f, mock_storage.brake_pedal_storage->prev_reading);
}

TEST_IN_TASK
void test_brake_pedal_fully_pressed_enables_brake(void) {
  /* normalized = (2000 - 1000) / 1000 = 1.0 → BRAKE_STATE_BRAKING */
  mock_raw_adc_reading = 2000;
  mock_storage.config->brake_low_pass_filter_alpha = 1.0f;
  mock_storage.brake_pedal_storage->prev_reading = 0.0f;

  TEST_ASSERT_OK(brake_pedal_run());

  TEST_ASSERT_EQUAL(BRAKE_STATE_BRAKING, mock_storage.brake_state);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, mock_storage.brake_pedal_storage->prev_reading);
}

TEST_IN_TASK
void test_brake_pedal_percentage_stored_in_storage(void) {
  /* normalized = (1500 - 1000) / 1000 = 0.5, LPF alpha=1.0 → 0.5 */
  mock_raw_adc_reading = 1500;
  TEST_ASSERT_OK(brake_pedal_run());

  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, mock_storage.brake_percentage);
  TEST_ASSERT_EQUAL(BRAKE_STATE_BRAKING, mock_storage.brake_state);
}
