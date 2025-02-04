#pragma once

/************************************************************************************************
 * @file   bms_carrier.h
 *
 * @brief  Header file for bms_carrier
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "i2c.h"
#include "ltc_afe.h"
#include "max17261_fuel_gauge.h"
#include "status.h"

/* Intra-component Headers */

/**
 * @defgroup bms_carrier
 * @brief    bms_carrier Firmware
 * @{
 */

/* Forward declarations of all storages */

struct AuxSenseStorage;
struct FaultBpsStorage;
struct StateOfChargeStorage;
struct RelayStorage;

/**
 * @brief   Battery management system configuration data
 */
typedef struct {
  uint8_t series_count;
  uint8_t parallel_count;
  uint16_t pack_capacity;
} BmsConfig;

/**
 * @brief   Battery management system storage
 */
typedef struct {
  int32_t pack_current;   // mA
  uint32_t pack_voltage;  // mV
  uint16_t temperature;

  BmsConfig bms_config;
  LtcAfeStorage ltc_afe_storage;
  Max17261Settings fuel_guage_settings;
  Max17261Storage fuel_guage_storage;

  struct AuxSenseStorage *aux_sense_storage;
  struct FaultBpsStorage *fault_bps_storage;
  struct StateOfChargeStorage *state_of_charge_storage;
  struct RelayStorage *relay_storage;
} BmsStorage;

/**
 * @brief   Initialize the battery-management system
 * @param   storage Pointer to the BMS storage
 * @param   config Pointer to the BMS config
 * @return  STATUS_CODE_OK if BMS initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode bms_carrier_init(BmsStorage *storage, BmsConfig *config);

/** @} */
