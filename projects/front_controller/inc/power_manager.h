#pragma once

/************************************************************************************************
 * @file    power_manager.h
 *
 * @brief   Power Control Manager
 *
 * @date    2025-07-27
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "front_controller.h"
#include "power_outputs.h"

/**
 * @defgroup Front_Controller
 * @brief    Front Controller Board Firmware
 * @{
 */

typedef struct PowerManagerStorage {
    uint16_t current_readings[NUM_OUTPUTS];
} PowerManagerStorage;

/**
 * @brief   Initialize all the GPIO pins
 * @param   storage Pointer to the front controller storage
 * @return  STATUS_CODE_OK if initialized succesfully
 *          STATUS_CODE_INVALID_ARGS if an invalid argument is passed in
 */
StatusCode power_manager_init(FrontControllerStorage *storage);

/**
 * @brief   Set an output group as active or inactive
 */
StatusCode power_manager_set_output_group(OutputGroup group, bool enable);

/**
 * @brief   Toggle an output group
 */
StatusCode power_manager_toggle_output_group(OutputGroup group);

/** @} */
