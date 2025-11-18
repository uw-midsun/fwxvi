#pragma once

/************************************************************************************************
 * @file   power_path_manager.h
 *
 * @brief  Power path manager class for the rear controller board
 *
 * @date   2025-05-28
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"
#include "status.h"

/* Intra-component Headers */

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

#define ZERO_OFFSET_MV 30
#define SENSITIVITY_MV_PER_A 132  // 40 * Vcc
#define R1 10000
#define R2 3300

bool check_valid(GpioAddress pin_address);

StatusCode power_path_manager_run(void);

StatusCode power_path_manager_init(RearControllerStorage *storage);


/** @} */
