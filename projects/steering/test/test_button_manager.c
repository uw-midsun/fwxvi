/************************************************************************************************
 * @file   test_button_manager.c
 *
 * @brief  Test file for button manager
 *
 * @date   2025-07-28
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "button_manager.h"

/* --- Mock GPIO Implementation --- */
static GpioState mock_gpio_states[BUTTON_MANAGER_MAX_BUTTONS];

GpioState TEST_MOCK(gpio_get_state)(const GpioAddress *addr) {
  return mock_gpio_states[addr->pin];
}

/* --- Callback Flags & Functions --- */
static bool pressed_called;
static bool released_called;
static bool btn0_pressed;
static bool btn1_released;

void test_press_callback(Button *btn) {
  (void)btn;
  pressed_called = true;
}
void test_release_callback(Button *btn) {
  (void)btn;
  released_called = true;
}
void test_btn0_press(Button *btn) {
  (void)btn;
  btn0_pressed = true;
}
void test_btn1_release(Button *btn) {
  (void)btn;
  btn1_released = true;
}

/* --- Button Manager & Configs --- */
static ButtonManager manager;

static ButtonConfig default_config[] = {
  { .gpio = { .port = GPIO_PORT_A, .pin = 0 }, .active_low = true, .debounce_ms = 2, .callbacks = { .rising_edge_cb = test_press_callback, .falling_edge_cb = test_release_callback } }
};

/* --- Setup/Teardown --- */
void setup_test(void) {
  pressed_called = false;
  released_called = false;
  btn0_pressed = false;
  btn1_released = false;
  for (uint8_t i = 0; i < BUTTON_MANAGER_MAX_BUTTONS; i++) {
    mock_gpio_states[i] = GPIO_STATE_HIGH;
  }

  TEST_ASSERT_OK(button_manager_init(&manager, default_config, 1));
}

void teardown_test(void) {
  /* nothing to clean up */
}

void test_button_press_and_release(void) {
  // Simulate press and release with debounce_ms = 2
  for (int i = 0; i < 7; ++i) {
    if (i == 1) mock_gpio_states[0] = GPIO_STATE_LOW;   // Simulate press
    if (i == 4) mock_gpio_states[0] = GPIO_STATE_HIGH;  // Simulate release
    TEST_ASSERT_OK(button_manager_update(&manager));
  }

  TEST_ASSERT_TRUE_MESSAGE(pressed_called, "Press callback was not called");
  TEST_ASSERT_TRUE_MESSAGE(released_called, "Release callback was not called");
}

void test_multi_button_independent_callbacks(void) {
  ButtonConfig multi_configs[2] = { { .gpio = { .port = 0, .pin = 0 }, .active_low = true, .debounce_ms = 1, .callbacks = { .rising_edge_cb = test_btn0_press, .falling_edge_cb = NULL } },
                                    { .gpio = { .port = 0, .pin = 1 }, .active_low = true, .debounce_ms = 1, .callbacks = { .rising_edge_cb = NULL, .falling_edge_cb = test_btn1_release } } };

  TEST_ASSERT_OK(button_manager_init(&manager, multi_configs, 2));

  // Simulate press on button 0
  mock_gpio_states[0] = GPIO_STATE_LOW;
  TEST_ASSERT_OK(button_manager_update(&manager));
  TEST_ASSERT_TRUE_MESSAGE(btn0_pressed, "Button0 press callback not called");
  TEST_ASSERT_FALSE_MESSAGE(btn1_released, "Button1 release should NOT have fired");

  // Simulate full press + release on button 1
  mock_gpio_states[1] = GPIO_STATE_LOW;
  TEST_ASSERT_OK(button_manager_update(&manager));
  TEST_ASSERT_OK(button_manager_update(&manager));

  mock_gpio_states[1] = GPIO_STATE_HIGH;
  TEST_ASSERT_OK(button_manager_update(&manager));
  TEST_ASSERT_OK(button_manager_update(&manager));

  TEST_ASSERT_TRUE_MESSAGE(btn1_released, "Button1 release callback not called");
}

void test_null_callbacks_no_crash(void) {
  ButtonConfig null_cfg = { .gpio = { .port = 0, .pin = 0 }, .active_low = true, .debounce_ms = 1, .callbacks = { .rising_edge_cb = NULL, .falling_edge_cb = NULL } };

  TEST_ASSERT_OK(button_manager_init(&manager, &null_cfg, 1));

  mock_gpio_states[0] = GPIO_STATE_LOW;
  TEST_ASSERT_OK(button_manager_update(&manager));
  mock_gpio_states[0] = GPIO_STATE_HIGH;
  TEST_ASSERT_OK(button_manager_update(&manager));

  TEST_PASS_MESSAGE("Null callbacks did not cause crash");
}

void test_short_bounce_no_trigger(void) {
  ButtonConfig bounce_cfg = { .gpio = { .port = 0, .pin = 0 }, .active_low = true, .debounce_ms = 3, .callbacks = { .rising_edge_cb = test_press_callback, .falling_edge_cb = test_release_callback } };

  TEST_ASSERT_OK(button_manager_init(&manager, &bounce_cfg, 1));

  mock_gpio_states[0] = GPIO_STATE_LOW;
  TEST_ASSERT_OK(button_manager_update(&manager));
  mock_gpio_states[0] = GPIO_STATE_HIGH;
  TEST_ASSERT_OK(button_manager_update(&manager));
  mock_gpio_states[0] = GPIO_STATE_LOW;
  TEST_ASSERT_OK(button_manager_update(&manager));
  mock_gpio_states[0] = GPIO_STATE_HIGH;
  TEST_ASSERT_OK(button_manager_update(&manager));

  TEST_ASSERT_FALSE_MESSAGE(pressed_called, "Press should NOT have fired on short bounce");
  TEST_ASSERT_FALSE_MESSAGE(released_called, "Release should NOT have fired on short bounce");
}
