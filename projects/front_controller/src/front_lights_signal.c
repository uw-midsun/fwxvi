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

static SoftTimer s_blink_signal_timer;
static SoftTimer s_blink_bps_timer;
static SteeringLightState current_steering_light_state = STEERING_LIGHTS_OFF_STATE;
static BpsLightState current_bps_light_state = BPS_LIGHT_OFF_STATE;

static void s_blink_signal_timer_callback(SoftTimerId timer_id) {
  switch (current_steering_light_state) {
    case STEERING_LIGHTS_LEFT_STATE:
      power_manager_toggle_output_group(OUTPUT_GROUP_LEFT_LIGHTS);
      break;

    case STEERING_LIGHTS_RIGHT_STATE:
      power_manager_toggle_output_group(OUTPUT_GROUP_RIGHT_LIGHTS);
      break;

    case STEERING_LIGHTS_HAZARD_STATE:
      power_manager_toggle_output_group(OUTPUT_GROUP_HAZARD_LIGHTS);
      break;

    default:
      break;
  }

  software_timer_reset(&s_blink_signal_timer);
}

static void s_blink_bps_timer_callback(SoftTimerId timer_id) {
  if (current_bps_light_state == BPS_LIGHT_ON_STATE) {
    power_manager_toggle_output_group(OUTPUT_GROUP_BPS_LIGHTS);
  }

  software_timer_reset(&s_blink_bps_timer);
}

StatusCode front_lights_signal_process_event(SteeringLightState new_state) {
  switch (new_state) {
    case STEERING_LIGHTS_OFF_STATE:
      if (current_steering_light_state != STEERING_LIGHTS_OFF_STATE) {
        software_timer_cancel(&s_blink_signal_timer);
      }
      current_steering_light_state = STEERING_LIGHTS_OFF_STATE;

      power_manager_set_output_group(OUTPUT_GROUP_LEFT_LIGHTS, false);
      power_manager_set_output_group(OUTPUT_GROUP_RIGHT_LIGHTS, false);
      break;

    case STEERING_LIGHTS_LEFT_STATE:
      if (current_steering_light_state != STEERING_LIGHTS_LEFT_STATE) {
        power_manager_set_output_group(OUTPUT_GROUP_RIGHT_LIGHTS, false);
      }

      if (current_steering_light_state == STEERING_LIGHTS_OFF_STATE) {
        software_timer_start(&s_blink_signal_timer);
      }
      current_steering_light_state = STEERING_LIGHTS_LEFT_STATE;
      break;

    case STEERING_LIGHTS_RIGHT_STATE:
      if (current_steering_light_state != STEERING_LIGHTS_RIGHT_STATE) {
        power_manager_set_output_group(OUTPUT_GROUP_LEFT_LIGHTS, false);
      }

      if (current_steering_light_state == STEERING_LIGHTS_OFF_STATE) {
        software_timer_start(&s_blink_signal_timer);
      }
      current_steering_light_state = STEERING_LIGHTS_RIGHT_STATE;
      break;

    case STEERING_LIGHTS_HAZARD_STATE:
      if (current_steering_light_state != STEERING_LIGHTS_HAZARD_STATE) {
        power_manager_set_output_group(OUTPUT_GROUP_HAZARD_LIGHTS, false);
      }
      if (current_steering_light_state == STEERING_LIGHTS_OFF_STATE) {
        software_timer_start(&s_blink_signal_timer);
      }
      current_steering_light_state = STEERING_LIGHTS_HAZARD_STATE;
      break;
    default:
      /* Invalid state */
      power_manager_set_output_group(OUTPUT_GROUP_LEFT_LIGHTS, false);
      power_manager_set_output_group(OUTPUT_GROUP_RIGHT_LIGHTS, false);
      break;
  }

  return STATUS_CODE_OK;
}

StatusCode front_lights_signal_set_bps_light(BpsLightState new_state) {
  if ((new_state == BPS_LIGHT_OFF_STATE) && (current_bps_light_state == BPS_LIGHT_ON_STATE)) {
    software_timer_cancel(&s_blink_bps_timer);
    power_manager_set_output_group(OUTPUT_GROUP_BPS_LIGHTS, false);
    current_bps_light_state = BPS_LIGHT_OFF_STATE;
  } else if ((new_state == BPS_LIGHT_ON_STATE) && (current_bps_light_state == BPS_LIGHT_OFF_STATE)) {
    software_timer_start(&s_blink_bps_timer);
    current_bps_light_state = BPS_LIGHT_ON_STATE;
  } else {
    return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}

StatusCode front_lights_signal_init(void) {
  status_ok_or_return(software_timer_init(FRONT_STEERING_LIGHTS_BLINK_PERIOD_MS, s_blink_signal_timer_callback, &s_blink_signal_timer));
  status_ok_or_return(software_timer_init(FRONT_BPS_LIGHTS_BLINK_PERIOD_MS, s_blink_bps_timer_callback, &s_blink_bps_timer));

  current_steering_light_state = STEERING_LIGHTS_OFF_STATE;

  return STATUS_CODE_OK;
}
