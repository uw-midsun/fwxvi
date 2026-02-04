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
#include "gpio.h"
#include "steering_hw_defs.h"

/**
 * @defgroup steering
 * @brief    steering Firmware
 * @{
 */

/**
 * @brief Light signal states representing the current output
 */
typedef enum {
  LIGHTS_SIGNAL_STATE_OFF = 0, /**< All lights off */
  LIGHTS_SIGNAL_STATE_LEFT,    /**< Left signal active */
  LIGHTS_SIGNAL_STATE_RIGHT,   /**< Right signal active */
  LIGHTS_SIGNAL_STATE_HAZARD   /**< Hazard signal active */
} LightsSignalState;

/**
 * @brief Requests made to control the light signals
 */
typedef enum {
  LIGHTS_SIGNAL_REQUEST_OFF = 0, /**< Request to turn off signals */
  LIGHTS_SIGNAL_REQUEST_LEFT,    /**< Request to turn on left signal */
  LIGHTS_SIGNAL_REQUEST_RIGHT,   /**< Request to turn on right signal */
  LIGHTS_SIGNAL_REQUEST_HAZARD   /**< Request to turn on hazard signal */
} LightsSignalRequest;

/**
 * @brief   Initialize state variables and timer
 */
void lights_signal_manager_init(void);

/**
 * @brief   Make a request to change the signal state
 * @param   req The requested light signal state
 */
void lights_signal_manager_request(LightsSignalRequest req);

/**
 * @brief   Update the signal state based on the current request
 */
void lights_signal_manager_update(void);

/**
 * @brief   Get the current state of the signal lights
 * @return  The current light signal state
 */
LightsSignalState lights_signal_manager_get_state(void);

/** @} */
