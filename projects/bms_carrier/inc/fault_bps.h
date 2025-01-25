#pragma once

/************************************************************************************************
 * @file   bms_hw_defs.h
 *
 * @brief  Header file for BMS hardware definitions
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "bms.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "status.h"

/* Intra-component Headers */

/**
 * @defgroup bms_carrier
 * @brief    bms_carrier Firmware
 * @{
 */

StatusCode fault_bps_init(BmsStorage *storage);

StatusCode fault_bps_set(uint8_t fault_bitmask);

StatusCode fault_bps_clear(uint8_t fault_bitmask);

uint8_t fault_bps_get(void);
typedef enum {
  BMS_FAULT_OVERVOLTAGE,
  BMS_FAULT_UNBALANCE,
  BMS_FAULT_OVERTEMP_AMBIENT,
  BMS_FAULT_COMMS_LOSS_AFE,
  BMS_FAULT_COMMS_LOSS_CURR_SENSE,
  BMS_FAULT_OVERTEMP_CELL,
  BMS_FAULT_OVERCURRENT,
  BMS_FAULT_UNDERVOLTAGE,
  BMS_FAULT_KILLSWITCH,
  BMS_FAULT_RELAY_CLOSE_FAILED,
  BMS_FAULT_DISCONNECTED
} BmsFault;
/** @} */
