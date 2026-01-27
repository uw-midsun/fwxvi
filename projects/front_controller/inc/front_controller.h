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
#include "global_enums.h"
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
struct Ws22MotorCanStorage;

/** @brief  Front controller pedal set to 2% deadzone  */
#define FRONT_CONTROLLER_ACCEL_INPUT_DEADZONE 0.02f

/** @brief  Front controller pedal is remapped from 0 - 1 to 0.1 to 1 */
#define FRONT_CONTROLLER_ACCEL_REMAP_MIN 0.10f

/** @brief  Front controller pedal exponent for non-linear feel */
#define FRONT_CONTROLLER_ACCEL_CURVE_EXPONENT 2.0f

/** @brief  Front controller accel pedal alpha value for low-pass filtering */
#define FRONT_CONTROLLER_ACCEL_LPF_ALPHA 0.25f

/** @brief  Front controller brake pedal alpha value for low-pass filtering */
#define FRONT_CONTROLLER_BRAKE_LPF_ALPHA 0.50f

/** @brief  Front controller brake pedal set to 1% deadzone  */
#define FRONT_CONTROLLER_BRAKE_INPUT_DEADZONE 0.01f

/** @brief Max velocity value used for ws22 motor controllers */
#define WS22_CONTROLLER_MAX_VELOCITY 12000

/** @brief Max speed of vehicle in KPH */
#define MAX_VEHICLE_SPEED_KPH 50

/**
 * @brief   Front Controller configuration data
 */
typedef struct {
  float accel_input_deadzone;        /**< Deadzone for accelerator input [0.0 - 1.0] */
  float accel_input_remap_min;       /**< Pedal input value mapped to zero torque (e.g. 0.2 to avoid dead torque zone) */
  float accel_input_curve_exponent;  /**< Exponent for non-linear pedal mapping (1.0 = linear, >1 = exponential) */
  float accel_low_pass_filter_alpha; /**< Alpha value for accel pedal low pass filter */
  float brake_pedal_deadzone;        /**< Deadzone for brake pedal input [0.0 - 1.0]  */
  float brake_low_pass_filter_alpha; /**< Alpha value for brake pedal low pass filter */
} FrontControllerConfig;

/**
 * @brief   Front Controller storage
 */
typedef struct {
  bool brake_enabled; /**< Brake enabled */
  bool regen_enabled; /**< Regen enabled */

  uint32_t vehicle_speed_kph; /**< Current vehicle speed in km/h */

  float accel_percentage;              /**< Acceleration pedal percentage after OPD algorithm and filtering is applied as a value between 0.0 - 1.0 */
  VehicleDriveState currentDriveState; /**< Current drive state of vehicle, determined by motor_can.c */

  struct PowerManagerStorage *power_manager_storage;  /**< Power manager storage */
  struct AccelPedalStorage *accel_pedal_storage;      /**< Acceleration pedal storage */
  struct BrakePedalStorage *brake_pedal_storage;      /**< Brake pedal storage */
  struct Ws22MotorCanStorage *ws22_motor_can_storage; /**< Wavesculptor 22 motor CAN storage */
  struct OpdStorage *opd_storage;                     /**< OPD storage */

  FrontControllerConfig *config; /**< Pointer to the front controller configuration data */
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
