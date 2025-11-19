/************************************************************************************************
 * @file    light_signal_manager.c
 *
 * @brief   Source file for light signal manager (turn signals, hazard)
 *
 * @date    2025-07-18
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "light_signal_manager.h"
/* Intra-component Headers */

static LightsSignalState current_state = LIGHTS_SIGNAL_STATE_OFF;
static LightsSignalRequest current_request = LIGHTS_SIGNAL_REQUEST_OFF;
static SoftTimer s_signal_blink_timer;
static bool s_is_led_on = false;

static void previous_set_signal_lights(bool turn_on) {
  GpioState state;

  if (turn_on == true) {
    state = GPIO_STATE_HIGH;
  } else {
    state = GPIO_STATE_LOW;
  }

  const GpioAddress left_led = STEERING_LEFT_TURN_LED;
  const GpioAddress right_led = STEERING_RIGHT_TURN_LED;

  if (current_state == LIGHTS_SIGNAL_STATE_LEFT) {
    gpio_set_state(&left_led, state);
    gpio_set_state(&right_led, GPIO_STATE_LOW);
  }
  else if (current_state == LIGHTS_SIGNAL_STATE_RIGHT) {
    gpio_set_state(&left_led, GPIO_STATE_LOW);
    gpio_set_state(&right_led, state);
  }
  else if (current_state == LIGHTS_SIGNAL_STATE_HAZARD) {
    gpio_set_state(&left_led, state);
    gpio_set_state(&right_led, state);
  } else {
    gpio_set_state(&left_led, GPIO_STATE_LOW);
    gpio_set_state(&right_led, GPIO_STATE_LOW);
  }
}

static void previous_timer_callback(SoftTimerId timer_id) {
  s_is_led_on = !s_is_led_on;
  previous_set_signal_lights(s_is_led_on);
  software_timer_init_and_start(LIGHT_SIGNAL_BLINK_PERIOD_MS, previous_timer_callback, &s_signal_blink_timer);
}
void lights_signal_manager_init(void) {
  current_state = LIGHTS_SIGNAL_STATE_OFF;
  current_request = LIGHTS_SIGNAL_REQUEST_OFF;
  software_timer_init(LIGHT_SIGNAL_BLINK_PERIOD_MS, previous_timer_callback, &s_signal_blink_timer);
}

void lights_signal_manager_update(void) {

  LightsSignalState previous_state = current_state;

  switch (current_request) {
    case LIGHTS_SIGNAL_REQUEST_OFF:
      current_state = LIGHTS_SIGNAL_STATE_OFF;
      break;
    case LIGHTS_SIGNAL_REQUEST_LEFT:
      if (current_state != LIGHTS_SIGNAL_STATE_HAZARD) {
        current_state = LIGHTS_SIGNAL_STATE_LEFT;
      }
      break;
    case LIGHTS_SIGNAL_REQUEST_RIGHT:
      if (current_state != LIGHTS_SIGNAL_STATE_HAZARD) {
        current_state = LIGHTS_SIGNAL_STATE_RIGHT;
      }
      break;
    case LIGHTS_SIGNAL_REQUEST_HAZARD:
      current_state = LIGHTS_SIGNAL_STATE_HAZARD;
      break;
  }

  if (current_state != previous_state) {
    if (current_state == LIGHTS_SIGNAL_STATE_OFF) {
      software_timer_cancel(&s_signal_blink_timer);
      s_is_led_on = false;
      previous_set_signal_lights(false);
    } else {
      s_is_led_on = true;
      previous_set_signal_lights(true);
      software_timer_init_and_start(LIGHT_SIGNAL_BLINK_PERIOD_MS, previous_timer_callback, &s_signal_blink_timer);
    }
  }
}

LightsSignalState lights_signal_manager_get_state(void) {
  return current_state;
}