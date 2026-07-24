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
 * @brief   Trigger a BPS fault with an associated cell index
 * @param   fault BPS Fault event
 * @param   cell_at_fault One-based cell index for cell-related faults, or 0 if not applicable
 * @return  STATUS_CODE_OK if triggered successfully
 *          STATUS_CODE_UNINITIALIZED if the system is not initialized
 */
StatusCode trigger_bps_fault_with_cell(BpsFault fault, uint8_t cell_at_fault);

/**
 * @brief   Trigger a BPS fault with a detail snapshot for the fault data payload
 * @details The detail is latched only for the first (root) fault while a fault is active.
 *          The fault code bitmask still accumulates every active fault bit.
 * @param   fault BPS Fault event
 * @param   cell_at_fault One-based cell index for cell-related faults, or 0 if not applicable
 * @param   data Fault-specific detail snapshot (see BpsFaultData in global_enums.h)
 * @return  STATUS_CODE_OK if triggered successfully
 *          STATUS_CODE_UNINITIALIZED if the system is not initialized
 *          STATUS_CODE_INVALID_ARGS if the fault is out of range
 */
StatusCode trigger_bps_fault_with_data(BpsFault fault, uint8_t cell_at_fault, BpsFaultData data);

/**
 * @brief   Clear the BPS fault
 * @param   fault BPS Fault event
 * @return  STATUS_CODE_OK if triggered successfully
 *          STATUS_CODE_UNINITIALIZED if the system is not initialized
 */
StatusCode bps_fault_clear();

/**
 * @brief   Whether BPS protection is disabled from steering (manual override)
 * @details Defaults to enabled until the first steering frame is received, so protection is never
 *          spuriously off at boot when the steering bitfield still reads its zero default.
 * @return  true if BPS is disabled, false otherwise
 */
bool bps_is_disabled(void);

/** @} */
