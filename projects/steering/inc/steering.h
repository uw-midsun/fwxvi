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
#include "global_enums.h"

/* Intra-component Headers */
#include "status.h"

/**
 * @defgroup steering
 * @brief    steering Firmware
 * @{
 */

/* Forward declarations */
struct ButtonManager;
struct ButtonLEDManager;

/** @brief  Steering button debounce period millisecond */
#define STEERING_BUTTON_DEBOUNCE_PERIOD_MS 5U

/** @brief  Maximum cruise control spesed in kilometers per hour*/
#define STEERING_CRUISE_MAX_SPEED_KMH 80U

/** @brief  Minimum cruise control speed in kilometers per hour */
#define STEERING_CRUISE_MIN_SPEED_KMH 40U

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

/**
 * @brief   Data for the display, CAN RX or internal
 */
typedef struct {
  float pedal_percentage;
  VehicleDriveState drive_state;
  bool brake_enabled;
  bool regen_enabled;

  uint16_t bps_fault;

  float motor_heatsink_temp;
  float motor_temp;

  float vehicle_velocity;
  float motor_velocity;

  uint32_t aux_voltage; /**< Auxiliary bus voltage (mV) */
  int32_t aux_current;  /**< Auxiliary bus current (mA) */

  uint32_t pack_voltage; /**< Pack voltage reading (mV) */
  int32_t pack_current;  /**< Pack current reading (mA) */
  float state_of_charge;
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

  struct ButtonManager *button_manager;        /**< Button manager */
  struct ButtonLEDManager *button_led_manager; /**< Button LED manager */
  DisplayData *display_data;                   /**< Pointer to data for the display */

  SteeringConfig *config; /**< Pointer to the steering configuration data */
} SteeringStorage;

/**
 * @brief   Initialize the steering interface
 * @param   storage Pointer to the steering storage
 * @param   config Pointer to the steering config
 * @return  STATUS_CODE_OK if steering initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode steering_init(SteeringStorage *storage, SteeringConfig *config);

/** @} */
