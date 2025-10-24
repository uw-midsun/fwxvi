#pragma once

/************************************************************************************************
 * @file    brake_pedal.h
 *
 * @brief   Header file for brake pedal manager
 *
 * @date    2025-09-15
 * @author  Midnight Sun Team #24 - MSXVI
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
} BrakePedalCalibrationData;

typedef struct BrakePedalStorage {
  float prev_reading;

  BrakePedalCalibrationData calibration_data;
} BrakePedalStorage;

/**
 * @brief   Reads the pedal percentage and applies deadzone and filtering before updating the brake state
 * @return  STATUS_CODE_OK if pedal is read successfully
 *          STATUS_CODE_UNINITIALIZED if pedal is not initialized
 *          STATUS_CODE_INTERNAL_ERROR if ADC fails
 */
StatusCode brake_pedal_run();

/**
 * @brief   Initializes the brake pedal hardware interface loads calibration data from memory
 * @return  STATUS_CODE_OK if pedal is initialized successfully
 *          STATUS_CODE_INVALID_ARGS if an invalid parameter is provided
 */
StatusCode brake_pedal_init(FrontControllerStorage *storage);

/** @} */
