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

#include "global_enums.h"
#include "steering_setters.h"
/* Intra-component Headers */

static SoftTimer s_signal_blink_timer;
static LightsSignalState current_state = LIGHTS_SIGNAL_STATE_OFF;
static LightsSignalRequest current_request = LIGHTS_SIGNAL_REQUEST_OFF;

const GpioAddress left_led = GPIO_STEERING_LEFT_TURN_LED;
const GpioAddress right_led = GPIO_STEERING_RIGHT_TURN_LED;

static void s_blink_timer_callback(SoftTimerId timer_id) {
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
  software_timer_init(GLOBAL_SIGNAL_LIGHTS_BLINK_PERIOD_MS, s_blink_timer_callback, &s_signal_blink_timer);
}

void lights_signal_manager_request(LightsSignalRequest req) {
  current_request = req;
}

void lights_signal_manager_update(void) {
  LightsSignalState previous_state = current_state;

  switch (current_request) {
    case LIGHTS_SIGNAL_REQUEST_OFF:
      if (previous_state != LIGHTS_SIGNAL_STATE_OFF) {
        set_steering_buttons_lights(STEERING_LIGHTS_OFF_STATE);
        gpio_set_state(&left_led, GPIO_STATE_LOW);
        gpio_set_state(&right_led, GPIO_STATE_LOW);

        current_state = LIGHTS_SIGNAL_STATE_OFF;
        software_timer_cancel(&s_signal_blink_timer);
      }
      break;

    case LIGHTS_SIGNAL_REQUEST_LEFT:
      if (previous_state != LIGHTS_SIGNAL_STATE_LEFT && previous_state != LIGHTS_SIGNAL_STATE_HAZARD) {
        set_steering_buttons_lights(STEERING_LIGHTS_LEFT_STATE);
        gpio_set_state(&left_led, GPIO_STATE_HIGH);
        gpio_set_state(&right_led, GPIO_STATE_LOW);

        current_state = LIGHTS_SIGNAL_STATE_LEFT;
        software_timer_start(&s_signal_blink_timer);
      }
      break;

    case LIGHTS_SIGNAL_REQUEST_RIGHT:
      if (previous_state != LIGHTS_SIGNAL_STATE_RIGHT && previous_state != LIGHTS_SIGNAL_STATE_HAZARD) {
        set_steering_buttons_lights(STEERING_LIGHTS_RIGHT_STATE);
        gpio_set_state(&left_led, GPIO_STATE_LOW);
        gpio_set_state(&right_led, GPIO_STATE_HIGH);

        current_state = LIGHTS_SIGNAL_STATE_RIGHT;
        software_timer_start(&s_signal_blink_timer);
      }
      break;

    case LIGHTS_SIGNAL_REQUEST_HAZARD:
      if (previous_state != LIGHTS_SIGNAL_STATE_HAZARD) {
        set_steering_buttons_lights(STEERING_LIGHTS_HAZARD_STATE);
        gpio_set_state(&left_led, GPIO_STATE_HIGH);
        gpio_set_state(&right_led, GPIO_STATE_HIGH);

        current_state = LIGHTS_SIGNAL_STATE_HAZARD;
        software_timer_start(&s_signal_blink_timer);
      }
      break;
  }
}

LightsSignalState lights_signal_manager_get_state(void) {
  return current_state;
}
