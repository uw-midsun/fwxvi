/***********************************************************************************************
 * @file    front_controller_state_manager.c
 *
 * @brief   Front Controller state machine manager source file
 *
 * @date    2025-09-14
 * @author  Midnight Sun Team #24 - MSXVI
 ***********************************************************************************************/

/* Standard library Headers */
#include <stdio.h>

/* Inter-component Headers */
#include "log.h"

/* Intra-component Headers */
#include "front_controller_state_manager.h"
#include "relays.h"

static FrontControllerStorage *front_controller_storage = NULL;
static FrontControllerState s_current_state = FRONT_CONTROLLER_STATE_INIT;

static void front_controller_state_manager_enter_state(FrontControllerState new_state) {
  switch (new_state) {
    case FRONT_CONTROLLER_STATE_INIT:
      /* TODO: Open all relays, reset internal flags */
      break;

    case FRONT_CONTROLLER_STATE_IDLE:
      /* TODO: Check if precharging the neg relay is done, then close negative relay */
      break;

    case FRONT_CONTROLLER_STATE_DRIVE:
      /* TODO: Check if motor precharge is done. If yes then close motor relay and enable LV motor supply. If not then do not engage, remain in idle */
      break;

    case FRONT_CONTROLLER_STATE_FAULT:
      /* TODO: Disable everything for safety, open all relays and disable LV motor */
      break;
  }

  s_current_state = new_state;
  LOG_DEBUG("FrontController State Manager entered state: %d\r\n", new_state);
}

StatusCode front_controller_state_manager_init(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  front_controller_storage = storage;

  front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_INIT);

  return STATUS_CODE_OK;
}

StatusCode front_controller_state_manager_step(FrontControllerEvent event) {
  if (front_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  switch (s_current_state) {
    case FRONT_CONTROLLER_STATE_INIT:
      if (event == FRONT_CONTROLLER_EVENT_INIT_COMPLETE)
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_IDLE);
      else if (event == FRONT_CONTROLLER_EVENT_FAULT)
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_FAULT);
      break;

    case FRONT_CONTROLLER_STATE_IDLE:
      if (event == FRONT_CONTROLLER_EVENT_DRIVE_REQUEST)
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_DRIVE);
      else if (event == FRONT_CONTROLLER_EVENT_FAULT)
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_FAULT);
      break;

    case FRONT_CONTROLLER_STATE_DRIVE:
      if (event == FRONT_CONTROLLER_EVENT_NEUTRAL_REQUEST) {
        // Open all relays and return to the safe INIT state.
        relays_reset();
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_INIT);
      } else if (event == FRONT_CONTROLLER_EVENT_FAULT) {
        front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_FAULT);
      }
      break;

    case FRONT_CONTROLLER_STATE_FAULT:
      if (event == FRONT_CONTROLLER_EVENT_RESET) front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_INIT);
      break;

    default:
      front_controller_state_manager_enter_state(FRONT_CONTROLLER_STATE_FAULT);
      break;
  }

  return STATUS_CODE_OK;
}

FrontControllerState front_controller_state_manager_get_state(void) {
  return s_current_state;
}
