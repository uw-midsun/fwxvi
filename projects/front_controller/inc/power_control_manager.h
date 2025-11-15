#pragma once

/************************************************************************************************
 * @file    power_control_manager.h
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
#include "power_control_outputs.h"

/**
 * @defgroup power_control_manager
 * @brief    power_control_manager Firmware
 * @{
 */

// To initialize all the GPIO outputs
void power_control_manager_init();

void power_control_set_output_group(OutputGroup group, bool enable);
/** @} */
