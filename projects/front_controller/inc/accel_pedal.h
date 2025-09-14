#pragma once

/************************************************************************************************
 * @file   accel_pedal.h
 *
 * @brief  Header file for acceleration pedal manager
 *
 * @date   2025-02-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "front_controller.h"

/**
 * @defgroup Front_Controller
 * @brief    Front Controller Board Firmware
 * @{
 */

typedef struct {
  uint16_t lower_value; /**< ADC reading when the pedal is considered fully released */
  uint16_t upper_value; /**< ADC reading when the pedal is considered fully pressed */
} AccelPedalCalibrationData;

typedef struct AccelPedalStorage {
  float accel_percentage;
  float prev_accel_percentage;

  AccelPedalCalibrationData calibration_data;
} AccelPedalStorage;

/**
 * @brief   Reads the pedal percentage and applies deadzone and filtering before updating the storage
 * @return  STATUS_CODE_OK if pedal is read succesfully
 *          STATUS_CODE_UNINITIALIZED if pedal is not initialized
 *          STATUS_CODE_INTERNAL_ERROR if ADC fails
 */
StatusCode accel_pedal_run();

/**
 * @brief   Initializes the acceleration pedal hardware interface loads calibration data from memory
 * @return  STATUS_CODE_OK if pedal is initialized succesfully
 *          STATUS_CODE_INVALID_ARGS if an invalid parameter is provided
 */
StatusCode accel_pedal_init(FrontControllerStorage *storage);

/** @} */
