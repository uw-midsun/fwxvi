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

const GpioAddress left_led = STEERING_LEFT_TURN_LED;
const GpioAddress right_led = STEERING_RIGHT_TURN_LED;

static void previous_timer_callback(SoftTimerId timer_id) {
  if (current_state == LIGHTS_SIGNAL_STATE_LEFT) {
    gpio_toggle_state(&left_led);
  } else if (current_state == LIGHTS_SIGNAL_STATE_RIGHT) {
    gpio_toggle_state(&right_led);
  } else if (current_state == LIGHTS_SIGNAL_STATE_HAZARD) {
    gpio_toggle_state(&left_led);
    gpio_toggle_state(&right_led);
  }
  software_timer_reset(&s_signal_blink_timer);
}

void lights_signal_manager_init(void) {
  current_state = LIGHTS_SIGNAL_STATE_OFF;
  current_request = LIGHTS_SIGNAL_REQUEST_OFF;
  software_timer_init(LIGHT_SIGNAL_BLINK_PERIOD_MS, previous_timer_callback, &s_signal_blink_timer);
}

void lights_signal_manager_request(LightsSignalRequest req) {
  current_request = req;
  software_timer_init(LIGHT_SIGNAL_BLINK_PERIOD_MS, previous_timer_callback, &s_signal_blink_timer);
}

void lights_signal_manager_update(void) {
  LightsSignalState previous_state = current_state;

  switch (current_request) {
    case LIGHTS_SIGNAL_REQUEST_OFF:
    if (previous_state != LIGHTS_SIGNAL_STATE_OFF) {
      current_state = LIGHTS_SIGNAL_STATE_OFF;
      software_timer_cancel(&s_signal_blink_timer);
      gpio_set_state(&left_led, GPIO_STATE_LOW);
      gpio_set_state(&right_led, GPIO_STATE_LOW);
    }
      break;

    case LIGHTS_SIGNAL_REQUEST_LEFT:
      if (previous_state != LIGHTS_SIGNAL_STATE_LEFT && previous_state != LIGHTS_SIGNAL_STATE_HAZARD) {
        current_state = LIGHTS_SIGNAL_STATE_LEFT;
        gpio_set_state(&left_led, GPIO_STATE_HIGH);
        gpio_set_state(&right_led, GPIO_STATE_LOW);
        software_timer_init_and_start(LIGHT_SIGNAL_BLINK_PERIOD_MS, previous_timer_callback, &s_signal_blink_timer);
      }
      break;
      
    case LIGHTS_SIGNAL_REQUEST_RIGHT:
      if (previous_state != LIGHTS_SIGNAL_STATE_RIGHT && previous_state != LIGHTS_SIGNAL_STATE_HAZARD) {
        current_state = LIGHTS_SIGNAL_STATE_RIGHT;
        gpio_set_state(&left_led, GPIO_STATE_LOW);
        gpio_set_state(&right_led, GPIO_STATE_HIGH);
        software_timer_init_and_start(LIGHT_SIGNAL_BLINK_PERIOD_MS, previous_timer_callback, &s_signal_blink_timer);
      }
      break;

    case LIGHTS_SIGNAL_REQUEST_HAZARD:
    if (previous_state != LIGHTS_SIGNAL_STATE_HAZARD) {
      current_state = LIGHTS_SIGNAL_STATE_HAZARD;
      gpio_set_state(&left_led, GPIO_STATE_HIGH);
      gpio_set_state(&right_led, GPIO_STATE_HIGH);
      software_timer_init_and_start(LIGHT_SIGNAL_BLINK_PERIOD_MS, previous_timer_callback, &s_signal_blink_timer);
    }
      break;
  }
}

LightsSignalState lights_signal_manager_get_state(void) {
  return current_state;
}
