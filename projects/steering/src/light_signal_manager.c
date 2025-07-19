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

void lights_signal_manager_init(void) {
  current_state = LIGHTS_SIGNAL_STATE_OFF;
  current_request = LIGHTS_SIGNAL_REQUEST_OFF;
}

void lights_signal_manager_request(LightsSignalRequest req) {
  current_request = req;
}

void lights_signal_manager_update(void) {
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
}

LightsSignalState lights_signal_manager_get_state(void) {
  return current_state;
}
