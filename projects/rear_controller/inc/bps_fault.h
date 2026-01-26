#pragma once

/************************************************************************************************
 * @file   bps_fault.h
 *
 * @brief  BPS Fault header file
 *
 * @date   2025-09-20
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "global_enums.h"

/* Intra-component Headers */
#include "rear_controller.h"

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

/**
 * @brief   Initialize the BPS fault memory
 * @param   storage Pointer to the rear controller storage
 * @return  STATUS_CODE_OK if initialized successfully
 *          STATUS_CODE_INVALID_ARGS if invalid parameter is passed in
 */
StatusCode bps_fault_init(RearControllerStorage *storage);

/**
 * @brief   Commit a new BPS fault to persist memory and CAN data
 * @return  STATUS_CODE_OK if committed successfully
 *          STATUS_CODE_UNINITIALIZED if the system is not initialized
 */
StatusCode bps_fault_commit(void);

/**
 * @brief   Trigger a BPS fault. This opens the relays and updates rear controller storage
 * @param   fault BPS Fault event
 * @return  STATUS_CODE_OK if triggered successfully
 *          STATUS_CODE_UNINITIALIZED if the system is not initialized
 */
StatusCode trigger_bps_fault(BpsFault fault);

/**
 * @brief   Clear the BPS fault
 * @param   fault BPS Fault event
 * @return  STATUS_CODE_OK if triggered successfully
 *          STATUS_CODE_UNINITIALIZED if the system is not initialized
 */
StatusCode bps_fault_clear();

/** @} */
