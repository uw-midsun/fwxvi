/***********************************************************************************************
 * @file    rear_controller_state_manager.c
 *
 * @brief   Rear Controller state machine manager source file
 *
 * @date    2025-09-14
 * @author  Midnight Sun Team #24 - MSXVI
 ***********************************************************************************************/

/* Standard library Headers */
#include <stdio.h>

/* Inter-component Headers */
#include "log.h"
#include "relays.h"
/* Intra-component Headers */
#include "rear_controller_safety_limits.h"
#include "rear_controller_state_manager.h"

static bool s_is_initialized = false;
static RearControllerState s_current_state = REAR_CONTROLLER_STATE_INIT;

static void rear_controller_state_manager_enter_state(RearControllerState new_state) {
  switch (new_state) {
    case REAR_CONTROLLER_STATE_INIT:
      relays_fault();

      /* TODO: Open all relays, reset internal flags */
      break;

    case REAR_CONTROLLER_STATE_PRECHARGE:
      relays_close_pos();
        /* TODO: Close positive relay and allow it to precharge */
      break;

    case REAR_CONTROLLER_STATE_IDLE:
      relays_close_neg();
      /* TODO: Check if precharging the neg relay is done, then close negative relay */
      break;

    case REAR_CONTROLLER_STATE_DRIVE:
      relays_close_motor();
      relays_close_solar();
      /* TODO: Check if motor precharge is done. If yes then close motor relay and enable LV motor supply. If not then do not engage, remain in idle */
      break;

    case REAR_CONTROLLER_STATE_CHARGE:
      relays_close_motor();
      /* TODO: Make any changes required for charging. Previous car required us to close motor relay */
      break;

    case REAR_CONTROLLER_STATE_FAULT:
      relays_fault();
      /* TODO: Disable everything for safety, open all relays and disable LV motor */
      break;
  }

  s_current_state = new_state;
  LOG_DEBUG("RearController State Manager entered state: %d\n", new_state);
}

StatusCode rear_controller_state_manager_init(void) {
  if (s_is_initialized) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_INIT);
  s_is_initialized = true;

  return STATUS_CODE_OK;
}

StatusCode rear_controller_state_manager_step(RearControllerEvent event) {
  if (!s_is_initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  switch (s_current_state) {
    case REAR_CONTROLLER_STATE_INIT:
      if (event == REAR_CONTROLLER_EVENT_INIT_COMPLETE)
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_PRECHARGE);
      else if (event == REAR_CONTROLLER_EVENT_FAULT)
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_FAULT);
      break;

    case REAR_CONTROLLER_STATE_PRECHARGE:
      if (event == REAR_CONTROLLER_EVENT_PRECHARGE_SUCCESS)
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_IDLE);
      else if (event == REAR_CONTROLLER_EVENT_PRECHARGE_FAIL || event == REAR_CONTROLLER_EVENT_FAULT)
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_FAULT);
      break;

    case REAR_CONTROLLER_STATE_IDLE:
      if (event == REAR_CONTROLLER_EVENT_DRIVE_REQUEST)
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_DRIVE);
      else if (event == REAR_CONTROLLER_EVENT_CHARGE_REQUEST)
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_CHARGE);
      else if (event == REAR_CONTROLLER_EVENT_FAULT)
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_FAULT);
      break;

    case REAR_CONTROLLER_STATE_DRIVE:
      if (event == REAR_CONTROLLER_EVENT_NEUTRAL_REQUEST)
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_IDLE);
      else if (event == REAR_CONTROLLER_EVENT_FAULT)
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_FAULT);
      break;

    case REAR_CONTROLLER_STATE_CHARGE:
      if (event == REAR_CONTROLLER_EVENT_CHARGER_REMOVED)
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_IDLE);
      else if (event == REAR_CONTROLLER_EVENT_FAULT)
        rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_FAULT);
      break;

    case REAR_CONTROLLER_STATE_FAULT:
      if (event == REAR_CONTROLLER_EVENT_RESET) rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_INIT);
      break;

    default:
      //Shouldnt reach here
      rear_controller_state_manager_enter_state(REAR_CONTROLLER_STATE_FAULT);
      break;
  }

  return STATUS_CODE_OK;
}

RearControllerState rear_controller_state_manager_get_state(void) {
  return s_current_state;
}
