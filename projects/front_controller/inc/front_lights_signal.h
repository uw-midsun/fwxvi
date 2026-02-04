#pragma once

/************************************************************************************************
 * @file    front_lights_signal.h
 *
 * @brief   Header file for front controller light signal receiver
 *
 * @date    2025-11-20
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "front_controller_hw_defs.h"

/**
 * @defgroup front_lights_signal
 * @brief    front_lights_signal Firmware
 * @{
 */

/**
 * @brief Initializes the front lights signal module
 */
StatusCode front_lights_signal_init();

/**
 * @brief Process recieved signal state
 * @param new_state which is the state recieved by the CAN message
 */
StatusCode front_lights_signal_process_event(SteeringLightState new_state);

/**
 * @brief Set the bps light
 * @param new_state which is the state recieved by the CAN message
 */
StatusCode front_lights_signal_set_bps_light(BpsLightState new_state);

/** @} */
