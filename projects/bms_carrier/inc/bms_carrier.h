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

#define CELL_CAPACITY_MAH 4850
#define NUM_SERIES_CELLS (4 * 9)
#define NUM_PARALLEL_CELLS (8)
#define PACK_CAPACITY_MAH (CELL_CAPACITY_MAH * NUM_PARALLEL_CELLS)

typedef struct {
  uint8_t series_count;
  uint8_t parallel_count;
  uint16_t pack_capacity;
} BmsConfig;

typedef struct {
  int32_t pack_current;   // mA
  uint32_t pack_voltage;  // mV
  uint16_t temperature;
  uint16_t aux_batt_voltage;  // mV
  uint16_t fault_bitset;
  BmsConfig config;

  LtcAfeStorage ltc_afe_storage;
  Max17261Settings fuel_guage_settings;
  Max17261Storage fuel_guage_storage;
} BmsStorage;

/** @} */
