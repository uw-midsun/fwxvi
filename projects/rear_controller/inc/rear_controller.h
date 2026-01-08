#pragma once

/************************************************************************************************
 * @file   rear_controller.h
 *
 * @brief  Header file for rear_controller
 *
 * @date   2025-05-28
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "adbms_afe.h"
#include "current_acs37800.h"

/* Intra-component Headers */

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

#define REAR_CONTROLLER_PRECHARGE_EVENT 0U
#define REAR_CONTROLLER_KILLSWITCH_EVENT 1U
#define REAR_CONTROLLER_CURRENT_SENSE_FILTER_ALPHA 0.5
#define REAR_CONTROLLER_CURRENT_SENSE_MAX_RETRIES 3

typedef struct {
  uint8_t series_count;   /**< Number of cells in series */
  uint8_t parallel_count; /**< Number of cells in parallel */
  float cell_capacity_Ah; /**< Nominal cell capacity [Ah] */
} RearControllerConfig;

typedef struct {
  int32_t pack_current;  /**< Pack current reading (mA) */
  uint32_t pack_voltage; /**< Pack voltage reading (mV) */
  uint32_t aux_voltage;  /**< Auxiliary bus voltage (mV) */
  uint32_t pcs_voltage;  /**< PCS bus voltage (mV) */
  int32_t aux_current;   /**< Auxiliary bus current (mA) */
  int32_t pcs_current;   /**< PCS bus current (mA) */

  float estimated_state_of_charge; /**< Estimated state of charge as a percentage [0 - 100] */

  /* Relay states */
  bool pos_relay_closed;   /**< Positive Relay closed state */
  bool neg_relay_closed;   /**< Negative Relay closed state */
  bool solar_relay_closed; /**< Solar Relay closed state */
  bool motor_relay_closed; /**< Motor Relay closed state */
  bool motor_lv_enabled;   /**< Motor LV enabled state */

  /* Safety monitors */
  bool precharge_complete; /**< Precharge complete state */
  bool killswitch_active;  /**< Killswitch active state */
  bool pcs_valid;          /**< PCS input valid state */
  bool aux_valid;          /**< Aux valid state */
  uint16_t bps_fault;      /**< Fault */

  /* Temperature monitoring */
  int16_t max_board_temperature;
  int16_t max_cell_temperature;

  AdbmsAfeStorage adbms_afe_storage; /**< ADBMS AFE storage */
  ACS37800Storage acs37800_storage;  /**< ACS37800 current sense storage */

  RearControllerConfig *config;
} RearControllerStorage;

StatusCode rear_controller_init(RearControllerStorage *storage, RearControllerConfig *config);

/** @} */
