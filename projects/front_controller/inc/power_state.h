#pragma once

/************************************************************************************************
 * @file   power_state.h
 *
 * @brief  Header file for power state manager
 *
 * @date   2025-11-26
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
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

/**
 * @brief   Sets the load switches based on states from steering and rear controller
 * @return  STATUS_CODE_OK
 */
StatusCode power_state_set_load_switches();
/**
 * @brief   Initializes the power state module and loads front controller storage from memory
 * @return  STATUS_CODE_OK if pedal is initialized successfully
 *          STATUS_CODE_INVALID_ARGS if an invalid parameter is provided
 */
StatusCode power_state_init(FrontControllerStorage *storage);

/** @} */
