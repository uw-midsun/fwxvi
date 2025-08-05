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
#include "unity.h"

/* Intra-component Headers */
#include "button_manager.h"

/* --- Mock GPIO --- */
static GpioState mock_gpio_states[BUTTON_MANAGER_MAX_BUTTONS];
GpioState gpio_get_state(const GpioAddress *addr) {
    (void)addr;
    return mock_gpio_states[addr->pin];
}

/* --- Callback flags & functions --- */
static bool pressed_called;
static bool released_called;
static bool btn0_pressed;
static bool btn1_released;

void test_press_callback(Button *btn)    { pressed_called  = true; }
void test_release_callback(Button *btn)  { released_called = true; }
void test_btn0_press(Button *btn)        { btn0_pressed    = true; }
void test_btn1_release(Button *btn)      { btn1_released   = true; }

/* --- ButtonConfig array --- */
static const ButtonConfig configs[] = {
    {
        .gpio        = { .port = 0, .pin = 0 },
        .active_low  = true,
        .debounce_ms = 2,
        .callbacks   = {
            .rising_edge_cb  = test_press_callback,
            .falling_edge_cb = test_release_callback
        }
    }
};

/* --- Shared ButtonManager --- */
static ButtonManager manager;

void setup_test(void) {
    pressed_called   = false;
    released_called  = false;
    btn0_pressed     = false;
    btn1_released    = false;
    for (uint8_t i = 0; i < BUTTON_MANAGER_MAX_BUTTONS; i++) {
        mock_gpio_states[i] = GPIO_STATE_HIGH;
    }
    button_manager_init(&manager, (ButtonConfig *)configs, 1);
}

void teardown_test(void) {
    /* nothing to clean up */
}

void test_button_press_and_release(void) {
    // i=0: idle
    // i=1: press → start debounce
    // i=2-3: stable LOW → press fires at end of i=3
    // i=4: release → start debounce
    // i=5-6: stable HIGH → release fires at end of i=6
    for (int i = 0; i < 7; ++i) {
        if (i == 1) mock_gpio_states[0] = GPIO_STATE_LOW;
        if (i == 4) mock_gpio_states[0] = GPIO_STATE_HIGH;
        button_manager_update(&manager);
    }
    TEST_ASSERT_TRUE_MESSAGE(pressed_called,  "Press callback was not called");
    TEST_ASSERT_TRUE_MESSAGE(released_called, "Release callback was not called");
}

void test_multi_button_independent_callbacks(void) {
    ButtonConfig multi_configs[2] = {
        {
            .gpio        = { .port = 0, .pin = 0 },
            .active_low  = true,
            .debounce_ms = 1,
            .callbacks   = { .rising_edge_cb = test_btn0_press, .falling_edge_cb = NULL }
        },
        {
            .gpio        = { .port = 0, .pin = 1 },
            .active_low  = true,
            .debounce_ms = 1,
            .callbacks   = { .rising_edge_cb = NULL,           .falling_edge_cb = test_btn1_release }
        }
    };
    button_manager_init(&manager, multi_configs, 2);

    // trigger rising edge on pin 0
    mock_gpio_states[0] = GPIO_STATE_LOW;
    button_manager_update(&manager);  // debounce_ms=1 → press fires
    TEST_ASSERT_TRUE_MESSAGE(btn0_pressed,   "Button0 press callback not called");
    TEST_ASSERT_FALSE_MESSAGE(btn1_released, "Button1 release should NOT have fired");

    // trigger falling edge on pin 1
    mock_gpio_states[1] = GPIO_STATE_LOW;
    button_manager_update(&manager);  // start LOW bounce (no callback)
    mock_gpio_states[1] = GPIO_STATE_HIGH;
    button_manager_update(&manager);  // raw change → reset debounce
    button_manager_update(&manager);  // stable HIGH → release fires
    TEST_ASSERT_TRUE_MESSAGE(btn1_released, "Button1 release callback not called");
}

void test_null_callbacks_no_crash(void) {
    ButtonConfig null_cfg = {
        .gpio        = { .port = 0, .pin = 0 },
        .active_low  = true,
        .debounce_ms = 1,
        .callbacks   = { .rising_edge_cb = NULL, .falling_edge_cb = NULL }
    };
    button_manager_init(&manager, &null_cfg, 1);

    // rapid toggle
    mock_gpio_states[0] = GPIO_STATE_LOW;
    button_manager_update(&manager);
    mock_gpio_states[0] = GPIO_STATE_HIGH;
    button_manager_update(&manager);

    // reaching here means no crash
    TEST_PASS_MESSAGE("Null-callback sequence did not crash");
}

void test_short_bounce_no_trigger(void) {
    ButtonConfig long_db_cfg = {
        .gpio        = { .port = 0, .pin = 0 },
        .active_low  = true,
        .debounce_ms = 3,
        .callbacks   = { .rising_edge_cb = test_press_callback,
                         .falling_edge_cb = test_release_callback }
    };
    button_manager_init(&manager, &long_db_cfg, 1);

    // bounce shorter than debounce_ms=3
    mock_gpio_states[0] = GPIO_STATE_LOW;   button_manager_update(&manager); // count=1
    mock_gpio_states[0] = GPIO_STATE_HIGH;  button_manager_update(&manager); // reset
    mock_gpio_states[0] = GPIO_STATE_LOW;   button_manager_update(&manager); // count=1
    mock_gpio_states[0] = GPIO_STATE_HIGH;  button_manager_update(&manager); // reset

    TEST_ASSERT_FALSE_MESSAGE(pressed_called,  "Press should NOT have fired on short bounce");
    TEST_ASSERT_FALSE_MESSAGE(released_called, "Release should NOT have fired on short bounce");
}