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
#include "global_enums.h"
#include "gpio.h"
#include "software_timer.h"

/* Intra-component Headers */
#include "front_lights_signal.h"
#include "power_manager.h"

static SoftTimer s_blink_timer;
static SteeringLightState current_state = STEERING_LIGHTS_OFF_STATE;

static void s_blink_timer_callback(SoftTimerId timer_id) {
  switch (current_state) {
    case STEERING_LIGHTS_LEFT_STATE:
      power_manager_toggle_output_group(LEFT_LIGHTS_GROUP);
      break;

    case STEERING_LIGHTS_RIGHT_STATE:
      power_manager_toggle_output_group(RIGHT_LIGHTS_GROUP);
      break;

    case STEERING_LIGHTS_HAZARD_STATE:
      power_manager_toggle_output_group(LEFT_LIGHTS_GROUP);
      power_manager_toggle_output_group(RIGHT_LIGHTS_GROUP);
      break;

    default:
      break;
  }

  software_timer_reset(&s_blink_timer);
}

StatusCode front_lights_signal_process_event(SteeringLightState new_state) {
  current_state = new_state;

  switch (current_state) {
    case STEERING_LIGHTS_OFF_STATE:
      software_timer_cancel(&s_blink_timer);
      power_manager_set_output_group(LEFT_LIGHTS_GROUP, false);
      power_manager_set_output_group(RIGHT_LIGHTS_GROUP, false);
      break;

    case STEERING_LIGHTS_LEFT_STATE:
      software_timer_start(&s_blink_timer);
      power_manager_set_output_group(LEFT_LIGHTS_GROUP, true);
      power_manager_set_output_group(RIGHT_LIGHTS_GROUP, false);
      break;

    case STEERING_LIGHTS_RIGHT_STATE:
      software_timer_start(&s_blink_timer);
      power_manager_set_output_group(LEFT_LIGHTS_GROUP, false);
      power_manager_set_output_group(RIGHT_LIGHTS_GROUP, true);
      break;

    case STEERING_LIGHTS_HAZARD_STATE:
      software_timer_start(&s_blink_timer);
      power_manager_set_output_group(LEFT_LIGHTS_GROUP, true);
      power_manager_set_output_group(RIGHT_LIGHTS_GROUP, true);
      break;
    default:
      /* Invalid state */
      power_manager_set_output_group(LEFT_LIGHTS_GROUP, false);
      power_manager_set_output_group(RIGHT_LIGHTS_GROUP, false);
      break;
  }

  return STATUS_CODE_OK;
}

StatusCode front_lights_signal_init(void) {
  status_ok_or_return(software_timer_init(FRONT_LIGHTS_BLINK_PERIOD_MS, s_blink_timer_callback, &s_blink_timer));

  current_state = STEERING_LIGHTS_OFF_STATE;

  return STATUS_CODE_OK;
}
