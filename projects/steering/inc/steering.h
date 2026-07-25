#pragma once

/************************************************************************************************
 * @file   steering.h
 *
 * @brief  Header file for steering
 *
 * @date   2025-07-09
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "display_defs.h"
#include "global_enums.h"
// #include "persist.h"
#include "ws22_motor_can.h"

/* Intra-component Headers */
#include "status.h"
#include "tasks.h"

/**
 * @defgroup steering
 * @brief    steering Firmware
 * @{
 */

/* Forward declarations */
struct ButtonManager;
struct ButtonLEDManager;
struct Ws22MotorCanConfig;

/** @brief  Steering button debounce period millisecond */
#define STEERING_BUTTON_DEBOUNCE_PERIOD_MS 5U

/** @brief  Maximum cruise control spesed in kilometers per hour*/
#define STEERING_CRUISE_MAX_SPEED_KMH 1000U  // 80

/** @brief  Minimum cruise control speed in kilometers per hour */
#define STEERING_CRUISE_MIN_SPEED_KMH 0U  // 40

/**
 * @brief   Steering buttons
 */
typedef enum {
  STEERING_BUTTON_HAZARDS,
  STEERING_BUTTON_DRIVE,
  STEERING_BUTTON_NEUTRAL,
  STEERING_BUTTON_REVERSE,

  STEERING_BUTTON_REGEN,

  STEERING_BUTTON_LEFT_LIGHT,
  STEERING_BUTTON_RIGHT_LIGHT,

  STEERING_BUTTON_HORN,

  STEERING_BUTTON_CRUISE_CONTROL_UP,
  STEERING_BUTTON_CRUISE_CONTROL_DOWN,

  NUM_STEERING_BUTTONS,
} SteeringButtons;

/**
 * @brief   Steering configuration data
 */
typedef struct {
  uint16_t cruise_min_speed_kmh; /**< Min cruise control speed in kilometers per hour */
  uint16_t cruise_max_speed_kmh; /**< Max cruise control speed in kilometers per hour */
} SteeringConfig;

// typedef struct __attribute__((aligned(4))) PersistData {
//   int64_t power_usage_wh;
// } PersistData;

/**
 * @brief   Data for the display, CAN RX or internal
 */
typedef struct {
  float state_of_charge;      /** Battery percentage from 0-100, two decimal points of precision*/
  uint16_t cell_voltages[36]; /**< All cell voltages */

  int16_t vehicle_velocity; /**< Velocity of the vehicle in kph */

  int16_t dcdc_voltage; /**< DC/DC or PCS bus voltage (mV) */
  int16_t dcdc_current; /**< DC/DC or PCS bus current (mA) */
  int16_t aux_voltage;  /**< Auxiliary bus voltage (mV) */
  int16_t aux_current;  /**< Auxiliary bus current (mA) */

  float pack_voltage;           /**< Pack voltage reading (V) */
  float pack_current;           /**< Pack current reading (A), signed (regen negative) */
  uint16_t min_cell_voltage_mv; /**< Minimum cell voltage reading (mV) */
  uint16_t max_cell_voltage_mv; /**< Maximum cell voltage reading (mV) */
  uint16_t max_cell_temp;       /**< Maximum cell temperature reading (C) */

  uint16_t thermistor_temp_c[NUMBER_OF_THERMISTORS]; /**< AFE thermistor temperatures (whole degrees C) from the AFE_temperature message */

  float energy_used_wh; /**< Net energy drawn from the pack since power-on (Wh), integrated on the display */

  uint16_t bps_fault;          /**< BPS fault bitfield */
  uint8_t bps_fault_cell;      /**< BPS fault cell number (if it exists) */
  bool bps_fault_live;         /**< TRUE if the fault is live (blocks drive); FALSE for a fault restored from flash */
  BpsFaultData bps_fault_data; /**< Fault detail snapshot from bps_fault_info CAN signal */

  VehicleDriveState drive_state;
  uint8_t pedal_percentage; /**< Pedal percentage, from (0, 100) */
  uint8_t brake_percentage; /**< Brake percentage, from (0, 100) */
  uint8_t brake_enabled;
  uint8_t regen_enabled;
  uint8_t precharge_complete;

  TickType_t display_rx_medium_last_start;
  int64_t power_usage;
} DisplayData;

/**
 * @brief   Steering storage
 */
typedef struct {
  bool horn_enabled;           /**< Horn enabled (set by horn button callback)*/
  bool cruise_control_enabled; /**< Cruise control enabled (set by cruise control button callback)*/

  uint16_t cruise_control_target_speed_kmh; /**< Cruise control target speed in kilometers per hour */
  uint8_t drive_state;                      /**< Drive state (see #DriveState) */
  uint8_t light_signal;                     /**< Light signal state (see #LightsSignalState) */

  struct ButtonManager *button_manager;               /**< Button manager */
  struct ButtonLEDManager *button_led_manager;        /**< Button LED manager */
  struct Ws22MotorCanStorage *ws22_motor_can_storage; /**< Wavesculptor 22 motor CAN storage */
  DisplayData display_data;                           /**< Data for the display */
  // PersistData persist_data;
  // PersistStorage *persist_storage;

  float estimated_km_remaining; /**< Estimated remaining range based on cell voltage */

  SteeringConfig *config;                    /**< Pointer to the steering configuration data */
  Ws22MotorCanConfig *ws22_motor_can_config; /**< Wavesculptor 22 motor CAN config - flags to indicate whether CAN for certain fields is enabled */
} SteeringStorage;

/**
 * @brief   Initialize the steering interface
 * @param   storage Pointer to the steering storage
 * @param   config Pointer to the steering config
 * @return  STATUS_CODE_OK if steering initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode steering_init(SteeringStorage *storage, SteeringConfig *config, Ws22MotorCanConfig *motor_can_config);

/**
 * @brief   Force BPS protection off (SECURE MODE OFF) regardless of the current state
 * @details Broadcasts the disabled BPS-enable state to the rear controller and syncs the menu's
 *          mirrored SECURE MODE label. Used when the driver acknowledges a BPS fault by leaving the
 *          fault takeover screen.
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode steering_force_disable_bps(void);

/** @} */
