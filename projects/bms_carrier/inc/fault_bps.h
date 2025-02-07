#pragma once

/************************************************************************************************
 * @file   fault_bps.h
 *
 * @brief  Header file for Battery protection system faults
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "bms_carrier.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "status.h"

/* Intra-component Headers */

/**
 * @defgroup bms_carrier
 * @brief    bms_carrier Firmware
 * @{
 */

/**
 * @brief   Fault storage for battery-protection system 
 */
struct FaultBpsStorage {
  uint16_t fault_bitset;    /**< Fault bitset */
};

/**
 * @brief   Initialize the Battery protection system faults
 * @param   bms_storage Pointer to the BMS storage
 * @return  STATUS_CODE_OK if BPS initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode fault_bps_init(BmsStorage *storage);

/**
 * @brief   Initialize the Battery protection system faults
 * @param   fault_bitmask Fault bitmask to set
 * @return  STATUS_CODE_OK if setting BPS fault succeeded
 *          STATUS_CODE_UNINITIALIZED if the Battery protection system is not initialized
 */
StatusCode fault_bps_set(uint8_t fault_bitmask);

/**
 * @brief   Initialize the Battery protection system faults
 * @param   fault_bitmask Fault bitmask to clear
 * @return  STATUS_CODE_OK if clearing BPS fault succeeded
 *          STATUS_CODE_UNINITIALIZED if the Battery protection system is not initialized
 */
StatusCode fault_bps_clear(uint8_t fault_bitmask);

/**
 * @brief   Initialize the Battery protection system faults
 * @return  Fault bitmask corresponding to BmsFault enum
 */
uint16_t fault_bps_get(void);

/** @} */
