#pragma once

/***********************************************************************************************
 * @file    rear_controller_state_manager.h
 *
 * @brief   Rear Controller state machine manager header file
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
#include "rear_controller.h"

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

typedef enum {
  REAR_CONTROLLER_STATE_INIT = 0,  /**< System init, checks and startup */
  REAR_CONTROLLER_STATE_PRECHARGE, /**< Precharge active, waiting for bus voltages to equalize */
  REAR_CONTROLLER_STATE_IDLE,      /**< Ready but not driving or charging, cars in neutral */
  REAR_CONTROLLER_STATE_DRIVE,     /**< Driving: motor relays closed, LV enabled */
  REAR_CONTROLLER_STATE_CHARGE,    /**< Charging: charger detected, motor relay may be required */
  REAR_CONTROLLER_STATE_FAULT      /**< Faulted: relays open, latched until reset */
} RearControllerState;

typedef enum {
  REAR_CONTROLLER_EVENT_NONE = 0,
  REAR_CONTROLLER_EVENT_INIT_COMPLETE,
  REAR_CONTROLLER_EVENT_PRECHARGE_SUCCESS,
  REAR_CONTROLLER_EVENT_PRECHARGE_FAIL,
  REAR_CONTROLLER_EVENT_DRIVE_REQUEST,
  REAR_CONTROLLER_EVENT_CHARGE_REQUEST,
  REAR_CONTROLLER_EVENT_NEUTRAL_REQUEST,
  REAR_CONTROLLER_EVENT_CHARGER_REMOVED,
  REAR_CONTROLLER_EVENT_FAULT,
  REAR_CONTROLLER_EVENT_RESET
} RearControllerEvent;

/**
 * @brief   Initialize the state machine
 * @param   storage Pointer to the rear controller storage
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_RESOURCE_EXHAUSTED if initialized already
 */
StatusCode rear_controller_state_manager_init(RearControllerStorage *storage);

/**
 * @brief   Advance the state machine based on inputs/events.
 * @param   event The triggering event
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_UNINITIALIZED if uninitialized
 */
StatusCode rear_controller_state_manager_step(RearControllerEvent event);

/**
 * @brief   Get current state.
 * @return  Current rear controller state (see #RearControllerState)
 */
RearControllerState rear_controller_state_manager_get_state(void);

/** @} */
