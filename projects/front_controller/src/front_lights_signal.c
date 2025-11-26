/************************************************************************************************
 * @file    front_lights_signal.c
 *
 * @brief   Source file for front controller light signal receiver which listens for CAN messages from steering and blinks front GPIOs
 *
 * @date    2025-11-20
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "front_lights_signal.h"

/* Intra-component Headers */

static SteeringLightState current_state = STEERING_LIGHTS_OFF_STATE;
static SoftTimer s_blink_timer;

// TODO: Update GPIO pins
static const GpioAddress s_front_left_light;
static const GpioAddress s_front_right_light;

void front_lights_signal_process_event(SteeringLightState new_state) {
  current_state = new_state;

  switch (current_state) {
    case STEERING_LIGHTS_OFF_STATE:
      software_timer_cancel(&s_blink_timer);
      gpio_set_state(&s_front_left_light, GPIO_STATE_LOW);
      gpio_set_state(&s_front_right_light, GPIO_STATE_LOW);
      break;

    case STEERING_LIGHTS_LEFT_STATE:
      software_timer_start(&s_blink_timer);
      gpio_set_state(&s_front_left_light, GPIO_STATE_HIGH);
      gpio_set_state(&s_front_right_light, GPIO_STATE_LOW);
      break;

    case STEERING_LIGHTS_RIGHT_STATE:
      software_timer_start(&s_blink_timer);
      gpio_set_state(&s_front_left_light, GPIO_STATE_LOW);
      gpio_set_state(&s_front_right_light, GPIO_STATE_HIGH);
      break;

    case STEERING_LIGHTS_HAZARD_STATE:
      software_timer_start(&s_blink_timer);
      gpio_set_state(&s_front_left_light, GPIO_STATE_HIGH);
      gpio_set_state(&s_front_right_light, GPIO_STATE_HIGH);
      break;
    default:
      // invalid state
      break;
  }
}

void front_lights_signal_init() {
  software_timer_init(FRONT_LIGHTS_BLINK_PERIOD_MS, previous_blink_timer_callback, &s_blink_timer);
  current_state = STEERING_LIGHTS_OFF_STATE;
}
