/************************************************************************************************
 * @file    button_led.c
 *
 * @brief   Source file for button led control
 *
 * @date    2026-01-28
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "button_led.h"

/* Intra-component Headers */

static LEDPixels rgb_led_colors[NUM_STEERING_BUTTONS] = {
  [STEERING_BUTTON_HAZARDS] = BUTTON_LED_MANAGER_COLOR_RED,

  [STEERING_BUTTON_DRIVE] = BUTTON_LED_MANAGER_COLOR_GREEN,
  [STEERING_BUTTON_NEUTRAL] = BUTTON_LED_MANAGER_COLOR_WHITE,
  [STEERING_BUTTON_REVERSE] = BUTTON_LED_MANAGER_COLOR_BLUE,

  [STEERING_BUTTON_REGEN] = BUTTON_LED_MANAGER_COLOR_CYAN,

  [STEERING_BUTTON_LEFT_LIGHT] = BUTTON_LED_MANAGER_COLOR_YELLOW,
  [STEERING_BUTTON_RIGHT_LIGHT] = BUTTON_LED_MANAGER_COLOR_YELLOW,

  [STEERING_BUTTON_HORN] = BUTTON_LED_MANAGER_COLOR_ORANGE,

  [STEERING_BUTTON_CRUISE_CONTROL_UP] = BUTTON_LED_MANAGER_COLOR_PURPLE,
  [STEERING_BUTTON_CRUISE_CONTROL_DOWN] = BUTTON_LED_MANAGER_COLOR_PINK,
};

static bool led_states[NUM_STEERING_BUTTONS] = {
  [STEERING_BUTTON_HAZARDS] = false,

  [STEERING_BUTTON_DRIVE] = false,
  [STEERING_BUTTON_NEUTRAL] = false,
  [STEERING_BUTTON_REVERSE] = false,

  [STEERING_BUTTON_REGEN] = false,

  [STEERING_BUTTON_LEFT_LIGHT] = false,
  [STEERING_BUTTON_RIGHT_LIGHT] = false,

  [STEERING_BUTTON_HORN] = false,

  [STEERING_BUTTON_CRUISE_CONTROL_UP] = false,
  [STEERING_BUTTON_CRUISE_CONTROL_DOWN] = false,
};

StatusCode button_led_enable(SteeringButtons button) {
  button_led_manager_set_color(button, rgb_led_colors[button]);
  led_states[button] = true;
  return STATUS_CODE_OK;
}

StatusCode button_led_disable(SteeringButtons button) {
  button_led_manager_set_color(button, (LEDPixels)BUTTON_LED_MANAGER_COLOR_OFF);
  led_states[button] = false;
  return STATUS_CODE_OK;
}

StatusCode button_led_toggle(SteeringButtons button) {
  if (led_states[button]) {
    button_led_disable(button);
    led_states[button] = false;
  } else {
    button_led_enable(button);
    led_states[button] = true;
  }
  return STATUS_CODE_OK;
}
