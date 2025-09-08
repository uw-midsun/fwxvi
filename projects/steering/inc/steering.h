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

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup steering
 * @brief    steering Firmware
 * @{
 */

/* Forward declarations */
struct ButtonManager;

#define STEERING_BUTTON_DEBOUNCE_PERIOD_MS 5U

/**
 * @brief   Steering buttons
 */
typedef enum {
  STEERING_BUTTON_LEFT_LIGHT,
  STEERING_BUTTON_RIGHT_LIGHT,
  STEERING_BUTTON_HAZARDS,

  STEERING_BUTTON_DRIVE,
  STEERING_BUTTON_REVERSE,
  STEERING_BUTTON_NEUTRAL,

  STEERING_BUTTON_HORN,

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
 * @brief   Steering storage
 */
typedef struct {
  bool horn_enabled;          /**< Horn enabled (set by horn button callback)*/
  bool regen_enabled;         /**< Regen enabled (set by regen button callback)*/
  bool cruse_control_enabled; /**< Cruise control enabled (set by cruise control button callback)*/

  uint16_t cruise_control_target_speed_kmh; /**< Cruise control target speed in kilometers per hour */
  uint8_t drive_state;                      /**< Drive state (see #DriveState) */
  uint8_t light_signal;                     /**< Light signal state (see #LightsSignalState) */

  struct ButtonManager *button_manager; /**< Aux sense storage */
  SteeringConfig *config;               /**< Pointer to the steering configuration data */
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
