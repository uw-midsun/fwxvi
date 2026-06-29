#pragma once

/************************************************************************************************
 * @file   curretn_sense.h
 *
 * @brief Current sensing for Rear_Controller
 *
 * @date   2025-09-17
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "rear_controller.h"

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

/**
 * @brief   Initializes the current sense sub-system
 * @param   storage Pointer to the rear controller storage
 * @return  STATUS_CODE_OK if initialized succesfully
 *          STATUS_CODE_INVALID_ARGS if invalid parameter is passed in
 */
StatusCode current_sense_init(RearControllerStorage *rear_controller_storage);

/**
 * @brief   Run a current sensing cycle to update pack voltage and pack current readings
 * @return  STATUS_CODE_OK if current sensed successfully
 *          STATUS_CODE_UNINITIALIZED if not initialized
 */
StatusCode current_sense_run();

/** @} */
