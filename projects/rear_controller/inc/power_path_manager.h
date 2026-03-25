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

/**
 * @brief   Initialize the power path manager
 * @param   storage Pointer to the rear controller storage
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_INVALID_ARGS if an invalid argument is passed
 */
StatusCode power_path_manager_init(RearControllerStorage *storage);

/**
 * @brief   Run power path manager calculations
 * @return  STATUS_CODE_OK if manager runs successfully
 */
StatusCode power_path_manager_run(void);

/** @} */
