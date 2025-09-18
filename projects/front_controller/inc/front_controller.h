#pragma once

/************************************************************************************************
 * @file   front_controller.h
 *
 * @brief  Header file for front_controller
 *
 * @date   2025-07-19
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */

/**
 * @defgroup Front_Controller
 * @brief    Front Controller Board Firmware
 * @{
 */

/* Forward declarations */
struct PowerSenseStorage;
struct AccelPedalStorage;

/** @brief  Front controller pedal set to 2% deadzone  */
#define FRONT_CONTROLLER_ACCEL_INPUT_DEADZONE 0.02f

/** @brief  Front controller pedal is remapped from 0 - 1 to 0.1 to 1 */
#define FRONT_CONTROLLER_ACCEL_REMAP_MIN 0.10f

/** @brief  Front controller pedal exponent for non-linear feel */
#define FRONT_CONTROLLER_ACCEL_CURVE_EXPONENT 2.0f

/** @brief  Front controller pedal alpha value for low-pass filtering */
#define FRONT_CONTROLLER_ACCEL_LPF_ALPHA 0.25f

/**
 * @brief   Front Controller configuration data
 */
typedef struct {
  float accel_input_deadzone;        /**< Deadzone for accelerator input [0.0 - 1.0] */
  float accel_input_remap_min;       /**< Pedal input value mapped to zero torque (e.g. 0.2 to avoid dead torque zone) */
  float accel_input_curve_exponent;  /**< Exponent for non-linear pedal mapping (1.0 = linear, >1 = exponential) */
  float accel_low_pass_filter_alpha; /**< Alpha value for accel pedal low pass filter */
} FrontControllerConfig;

/**
 * @brief   Front Controller storage
 */
typedef struct {
  bool brake_enabled; /**< Horn enabled (set by horn button callback) */

  uint32_t vehicle_speed_kph; /**< Current vehicle speed in km/hw */
  float accel_percentage;     /**< Acceleration pedal percentage after OPD algorithm and filtering is applied as a value between 0.0 - 1.0 */

  struct PowerSenseStorage *power_sense_storage; /**< Power sense storage */
  struct AccelPedalStorage *accel_pedal_storage; /**< Acceleration pedal storage */
  struct OpdStorage *opd_storage;
  FrontControllerConfig *config;                 /**< Pointer to the front controller configuration data */
} FrontControllerStorage;

/**
 * @brief   Initialize the front controller interface
 * @param   storage Pointer to the front controller storage
 * @param   config Pointer to the front controller config
 * @return  STATUS_CODE_OK if front controller initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode front_controller_init(FrontControllerStorage *storage, FrontControllerConfig *config);

/** @} */
