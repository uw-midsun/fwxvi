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

#include "button_led.h"
#include "button_led_manager.h"
#include "buzzer.h"
#include "global_enums.h"
#include "steering.h"
#include "steering_setters.h"
/* Intra-component Headers */

static SoftTimer s_signal_blink_timer;
static LightsSignalState current_state = LIGHTS_SIGNAL_STATE_OFF;
static LightsSignalCommand current_request = LIGHTS_SIGNAL_COMMAND_NONE;

static bool left_led_state = false;
static bool right_led_state = false;
static bool hazard_led_state = false;

static bool led_states[NUM_STEERING_BUTTONS] = { [STEERING_BUTTON_HAZARDS] = false, [STEERING_BUTTON_RIGHT_LIGHT] = false, [STEERING_BUTTON_LEFT_LIGHT] = false };

static void s_blink_timer_callback(SoftTimerId timer_id) {
  if (current_state == LIGHTS_SIGNAL_STATE_LEFT) {
    button_led_toggle(STEERING_BUTTON_LEFT_LIGHT);
  } else if (current_state == LIGHTS_SIGNAL_STATE_RIGHT) {
    button_led_toggle(STEERING_BUTTON_RIGHT_LIGHT);
  } else if (current_state == LIGHTS_SIGNAL_STATE_HAZARD) {
    button_led_toggle(STEERING_BUTTON_HAZARDS);
  }
  software_timer_reset(&s_signal_blink_timer);
}

void lights_signal_manager_init(void) {
  current_state = LIGHTS_SIGNAL_STATE_OFF;
  current_request = LIGHTS_SIGNAL_COMMAND_NONE;
  software_timer_init(GLOBAL_SIGNAL_LIGHTS_BLINK_PERIOD_MS, s_blink_timer_callback, &s_signal_blink_timer);
}

StatusCode lights_signal_manager_register(LightsSignalCommand req) {
  current_request = req;
  return STATUS_CODE_OK;
}

StatusCode lights_signal_manager_update(void) {
  LightsSignalState previous_state = current_state;

  switch (current_request) {
    case LIGHTS_SIGNAL_COMMAND_NONE:
      break;
    case LIGHTS_SIGNAL_COMMAND_LEFT:
      if (previous_state != LIGHTS_SIGNAL_STATE_LEFT && previous_state != LIGHTS_SIGNAL_STATE_HAZARD) {
        set_steering_buttons_lights(STEERING_LIGHTS_LEFT_STATE);
        button_led_disable(STEERING_BUTTON_RIGHT_LIGHT);
        software_timer_start(&s_signal_blink_timer);
        buzzer_start_turn_signal();

        current_state = LIGHTS_SIGNAL_STATE_LEFT;
        current_request = LIGHTS_SIGNAL_COMMAND_NONE;
      } else if (previous_state == LIGHTS_SIGNAL_STATE_LEFT) {
        set_steering_buttons_lights(STEERING_LIGHTS_OFF_STATE);
        software_timer_cancel(&s_signal_blink_timer);
        button_led_disable(STEERING_BUTTON_LEFT_LIGHT);
        button_led_disable(STEERING_BUTTON_RIGHT_LIGHT);
        buzzer_stop_turn_signal();

        current_state = LIGHTS_SIGNAL_STATE_OFF;
        current_request = LIGHTS_SIGNAL_COMMAND_NONE;
      }
      break;

    case LIGHTS_SIGNAL_COMMAND_RIGHT:
      if (previous_state != LIGHTS_SIGNAL_STATE_RIGHT && previous_state != LIGHTS_SIGNAL_STATE_HAZARD) {
        set_steering_buttons_lights(STEERING_LIGHTS_RIGHT_STATE);
        button_led_disable(STEERING_BUTTON_LEFT_LIGHT);
        software_timer_start(&s_signal_blink_timer);
        buzzer_start_turn_signal();

        current_state = LIGHTS_SIGNAL_STATE_RIGHT;
        current_request = LIGHTS_SIGNAL_COMMAND_NONE;
      } else if (previous_state == LIGHTS_SIGNAL_STATE_RIGHT) {
        set_steering_buttons_lights(STEERING_LIGHTS_OFF_STATE);
        software_timer_cancel(&s_signal_blink_timer);
        button_led_disable(STEERING_BUTTON_LEFT_LIGHT);
        button_led_disable(STEERING_BUTTON_RIGHT_LIGHT);
        buzzer_stop_turn_signal();

        current_state = LIGHTS_SIGNAL_STATE_OFF;
        current_request = LIGHTS_SIGNAL_COMMAND_NONE;
      }
      break;

    case LIGHTS_SIGNAL_COMMAND_HAZARD:
      if (previous_state != LIGHTS_SIGNAL_STATE_HAZARD) {
        set_steering_buttons_lights(STEERING_LIGHTS_HAZARD_STATE);
        button_led_disable(STEERING_BUTTON_LEFT_LIGHT);
        button_led_disable(STEERING_BUTTON_RIGHT_LIGHT);
        software_timer_start(&s_signal_blink_timer);
        buzzer_start_turn_signal();

        current_state = LIGHTS_SIGNAL_STATE_HAZARD;
        current_request = LIGHTS_SIGNAL_COMMAND_NONE;
      } else if (previous_state == LIGHTS_SIGNAL_STATE_HAZARD) {
        set_steering_buttons_lights(STEERING_LIGHTS_OFF_STATE);
        software_timer_cancel(&s_signal_blink_timer);
        button_led_disable(STEERING_BUTTON_HAZARDS);
        buzzer_stop_turn_signal();

        current_state = LIGHTS_SIGNAL_STATE_OFF;
        current_request = LIGHTS_SIGNAL_COMMAND_NONE;
      }
      break;
  }

  return STATUS_CODE_OK;
}

LightsSignalState lights_signal_manager_get_state(void) {
  return current_state;
}
