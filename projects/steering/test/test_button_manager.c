/************************************************************************************************
 * @file    test_button_manager.c
 *
 * @brief   Test file for button manager
 *
 * @date    2025-09-03
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/
/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "button_manager.h"
#include "drive_state_manager.h"
#include "light_signal_manager.h"
#include "steering.h"
#include "steering_hw_defs.h"

/* --- Constants --- */
#define DEBOUNCE_CYCLES (STEERING_BUTTON_DEBOUNCE_PERIOD_MS + 1)

/* --- Mock GPIO Implementation --- */
static GpioState mock_gpio_states[BUTTON_MANAGER_MAX_BUTTONS];

static LightsSignalState signal_requested;
static DriveState drive_requested;

GpioState TEST_MOCK(gpio_get_state)(const GpioAddress *addr) {
  return mock_gpio_states[addr->pin];
}

StatusCode TEST_MOCK(lights_signal_manager_request)(LightsSignalState state) {
  signal_requested = state;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(drive_state_manager_request)(DriveState state) {
  drive_requested = state;
  return STATUS_CODE_OK;
}

/* --- Button Manager Instance --- */
static ButtonManager manager;

/* --- Helpers --- */
static void simulate_updates(uint8_t cycles) {
  for (uint8_t i = 0; i < cycles; i++) {
    TEST_ASSERT_OK(button_manager_update(&manager));
  }
}

void setup_test(void) {
  signal_requested = LIGHTS_SIGNAL_REQUEST_OFF;
  drive_requested = DRIVE_STATE_INVALID;

  for (uint8_t i = 0; i < BUTTON_MANAGER_MAX_BUTTONS; i++) {
    mock_gpio_states[i] = GPIO_STATE_HIGH;
  }

  TEST_ASSERT_OK(button_manager_init(&manager));
}

void teardown_test(void) {
  /* nothing to clean up */
}

void test_left_turn_button_triggers_signal(void) {
  GpioAddress button_under_test = STEERING_LEFT_TURN_BUTTON;

  mock_gpio_states[button_under_test.pin] = GPIO_STATE_LOW;
  simulate_updates(DEBOUNCE_CYCLES);

  mock_gpio_states[button_under_test.pin] = GPIO_STATE_HIGH;
  simulate_updates(DEBOUNCE_CYCLES);

  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_STATE_LEFT, signal_requested);
}

void test_right_turn_button_triggers_signal(void) {
  GpioAddress button_under_test = STEERING_RIGHT_TURN_BUTTON;

  mock_gpio_states[button_under_test.pin] = GPIO_STATE_LOW;
  simulate_updates(DEBOUNCE_CYCLES);

  mock_gpio_states[button_under_test.pin] = GPIO_STATE_HIGH;
  simulate_updates(DEBOUNCE_CYCLES);

  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_STATE_RIGHT, signal_requested);
}

void test_hazards_button_triggers_signal(void) {
  GpioAddress button_under_test = STEERING_HAZARDS_BUTTON;

  mock_gpio_states[button_under_test.pin] = GPIO_STATE_LOW;
  simulate_updates(DEBOUNCE_CYCLES);

  mock_gpio_states[button_under_test.pin] = GPIO_STATE_HIGH;
  simulate_updates(DEBOUNCE_CYCLES);

  TEST_ASSERT_EQUAL(LIGHTS_SIGNAL_STATE_HAZARD, signal_requested);
}

void test_drive_button_triggers_drive_state(void) {
  GpioAddress button_under_test = STEERING_DRIVE_BUTTON;

  mock_gpio_states[button_under_test.pin] = GPIO_STATE_LOW;
  simulate_updates(DEBOUNCE_CYCLES);

  mock_gpio_states[button_under_test.pin] = GPIO_STATE_HIGH;
  simulate_updates(DEBOUNCE_CYCLES);

  TEST_ASSERT_EQUAL(DRIVE_STATE_DRIVE, drive_requested);
}

void test_reverse_button_triggers_drive_state(void) {
  GpioAddress button_under_test = STEERING_REVERSE_BUTTON;

  mock_gpio_states[button_under_test.pin] = GPIO_STATE_LOW;
  simulate_updates(DEBOUNCE_CYCLES);

  mock_gpio_states[button_under_test.pin] = GPIO_STATE_HIGH;
  simulate_updates(DEBOUNCE_CYCLES);

  TEST_ASSERT_EQUAL(DRIVE_STATE_REVERSE, drive_requested);
}

void test_neutral_button_triggers_drive_state(void) {
  GpioAddress button_under_test = STEERING_NEUTRAL_BUTTON;

  mock_gpio_states[button_under_test.pin] = GPIO_STATE_LOW;
  simulate_updates(DEBOUNCE_CYCLES);

  mock_gpio_states[button_under_test.pin] = GPIO_STATE_HIGH;
  simulate_updates(DEBOUNCE_CYCLES);

  TEST_ASSERT_EQUAL(DRIVE_STATE_NEUTRAL, drive_requested);
}

void test_short_bounce_on_button_does_not_trigger(void) {
  GpioAddress button_under_test = STEERING_DRIVE_BUTTON;

  mock_gpio_states[button_under_test.pin] = GPIO_STATE_LOW;
  simulate_updates(STEERING_BUTTON_DEBOUNCE_PERIOD_MS - 2);

  mock_gpio_states[button_under_test.pin] = GPIO_STATE_HIGH;
  simulate_updates(DEBOUNCE_CYCLES);

  TEST_ASSERT_EQUAL(DRIVE_STATE_INVALID, drive_requested);
}
