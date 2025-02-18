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

struct PrechargeSettings;
struct FanSettings;
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
  int32_t pack_current;  /**< Pack current reading */
  uint32_t pack_voltage; /**< Pack voltage reading */
  uint16_t temperature;  /**< Pack temperature reading */

  BmsConfig bms_config;                 /**< BMS configuration settings */
  LtcAfeStorage ltc_afe_storage;        /**< LTC6811 AFE Storage */
  Max17261Settings fuel_guage_settings; /**< MAX17261 Fuel gauge settings */
  Max17261Storage fuel_guage_storage;   /**< MAX17261 Fuel gauge storage */

  struct PrechargeSettings *precharge_settings;         /**< Precharge settings */
  struct FanSettings *fan_settings;                     /**< Fan settings */
  struct AuxSenseStorage *aux_sense_storage;            /**< Aux sense storage */
  struct FaultBpsStorage *fault_bps_storage;            /**< Fault BPS storage */
  struct StateOfChargeStorage *state_of_charge_storage; /**< State of charge storage */
  struct RelayStorage *relay_storage;                   /**< Relay storage */
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
