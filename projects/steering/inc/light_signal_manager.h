#pragma once

/************************************************************************************************
 * @file    light_signal_manager.h
 *
 * @brief   Header file for light signal manager (turn signals, hazard)
 *
 * @date    2025-07-16
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "software_timer.h"

/* Intra-component Headers */
#include "global_enums.h"
#include "gpio.h"
#include "steering.h"
#include "steering_hw_defs.h"

/**
 * @defgroup steering
 * @brief    steering Firmware
 * @{
 */

/**
 * @brief Requests made to control the light signals
 */
typedef enum {
  LIGHTS_SIGNAL_COMMAND_NONE = 0,   /**< No light signal command */
  LIGHTS_SIGNAL_COMMAND_LEFT = 1,   /**< Request to turn on left signal */
  LIGHTS_SIGNAL_COMMAND_RIGHT = 2,  /**< Request to turn on right signal */
  LIGHTS_SIGNAL_COMMAND_HAZARD = 3, /**< Request to turn on hazard signal */
  LIGHTS_SIGNAL_COMMAND_OFF = 4     /**< Request to turn off any active or pending signal */
} LightsSignalCommand;

/**
 * @brief   Initialize state variables and timer
 * @param   storage Pointer to the SteeringStorage instance
 */
void lights_signal_manager_init(SteeringStorage *storage);

/**
 * @brief   Make a request to change the signal state
 * @param   req The requested light signal state
 */
StatusCode lights_signal_manager_register(LightsSignalCommand req);

/**
 * @brief   Update the signal state based on the current request
 */
StatusCode lights_signal_manager_update(void);

/**
 * @brief   Get the current state of the signal lights
 * @return  The current light signal state
 */
SteeringLightState lights_signal_manager_get_state(void);

/** @} */
