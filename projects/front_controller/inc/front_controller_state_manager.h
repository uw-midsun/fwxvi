#pragma once

/***********************************************************************************************
 * @file    front_controller_state_manager.h
 *
 * @brief   Front Controller state machine manager header file
 *
 * @date    2025-09-14
 * @author  Midnight Sun Team #24 - MSXVI
 ***********************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "front_controller.h"

/**
 * @defgroup Front_Controller
 * @brief    Front Controller Board Firmware
 * @{
 */

typedef enum {
  FRONT_CONTROLLER_STATE_IDLE = 0, /**< Ready but not driving or charging, cars in neutral */
  FRONT_CONTROLLER_STATE_ENGAGED,  /**< Driving: motor relays closed, LV enabled */
  FRONT_CONTROLLER_STATE_FAULT,    /**< Faulted: relays open, latched until reset */
  NUM_FRONT_CONTROLLER_STATES
} FrontControllerState;

typedef enum {
  FRONT_CONTROLLER_EVENT_NONE = 0,
  FRONT_CONTROLLER_EVENT_IDLE_REQUEST,
  FRONT_CONTROLLER_EVENT_DRIVE_REQUEST,
  FRONT_CONTROLLER_EVENT_FAULT,
  FRONT_CONTROLLER_EVENT_RESET
} FrontControllerEvent;

/**
 * @brief   Initialize the state machine
 * @param   storage Pointer to the front controller storage
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_RESOURCE_EXHAUSTED if initialized already
 */
StatusCode front_controller_state_manager_init(FrontControllerStorage *storage);

/**
 * @brief   Advance the state machine based on inputs/events.
 * @param   event The triggering event
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_UNINITIALIZED if uninitialized
 */
StatusCode front_controller_state_manager_step(FrontControllerEvent event);

/**
 * @brief   Get current state.
 * @return  Current front controller state (see #FrontControllerState)
 */
FrontControllerState front_controller_state_manager_get_state(void);

StatusCode front_controller_update_state_manager_medium_cycle();
/** @} */
