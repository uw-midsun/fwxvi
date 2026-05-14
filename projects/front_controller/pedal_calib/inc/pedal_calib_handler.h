#pragma once

/************************************************************************************************
 * @file   pedal_calib_handler.h
 *
 * @brief  Handler for remote pedal calibration via CAN
 *
 * @date   2026-04-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "front_controller.h"

/**
 * @defgroup Front_Controller
 * @brief    Front Controller Board Firmware
 * @{
 */

/**
 * @brief Pedal calibration status/command values
 */
typedef enum {
  PEDAL_CALIB_STATUS_IDLE = 0,
  PEDAL_CALIB_STATUS_ACCEL_RAW_UNPRESSED = 1,
  PEDAL_CALIB_STATUS_ACCEL_RAW_PRESSED = 2,
  PEDAL_CALIB_STATUS_ACCEL_AMP_UNPRESSED = 3,
  PEDAL_CALIB_STATUS_ACCEL_AMP_PRESSED = 4,
  PEDAL_CALIB_STATUS_BRAKE_UNPRESSED = 5,
  PEDAL_CALIB_STATUS_BRAKE_PRESSED = 6,
  PEDAL_CALIB_STATUS_COMPLETE = 7,
  PEDAL_CALIB_STATUS_ERROR = 8,
} PedalCalibStatus;

/**
 * @brief   Initialize pedal calibration handler
 * @param   storage Pointer to the front controller storage
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode pedal_calib_handler_init(FrontControllerStorage *storage);

/**
 * @brief   Process pedal calibration request from CAN
 * @details Handles CAN messages from steering requesting calibration
 * @param   storage Pointer to the front controller storage
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode pedal_calib_handler_rx(FrontControllerStorage *storage);

/**
 * @brief   Execute one step of the calibration process
 * @param   storage Pointer to the front controller storage
 * @return  STATUS_CODE_OK on success, error otherwise
 */
StatusCode pedal_calib_handler_run(FrontControllerStorage *storage);

/** @} */
