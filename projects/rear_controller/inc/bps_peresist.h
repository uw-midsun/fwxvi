#pragma once

/************************************************************************************************
 * @file   bps_persist.h
 *
 * @brief  BPS Persist header file
 *
 * @date   2025-09-20
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "rear_controller.h"

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

/**
 * @brief   Initialize the BPS persist memory
 * @param   storage Pointer to the rear controller storage
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_INVALID_ARGS if invalid parameter is passed in
 */
StatusCode bps_persist_init(RearControllerStorage *storage);

/**
 * @brief   Commit a new BPS fault to persist memory
 * @return  STATUS_CODE_OK if committed successfully
 */
StatusCode bps_commit(void);

/** @} */
