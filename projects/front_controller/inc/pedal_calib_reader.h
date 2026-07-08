#pragma once

/************************************************************************************************
 * @file    pedal_calib_reader.h
 *
 * @brief   Pedal Calib Reader
 *
 * @date    2025-09-16
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

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
 * @brief   Apply the pedal persist data to the pedal storage calibration
 * @details Maps the values held in storage->pedal_persist_data (loaded from flash or freshly
 *          sampled during a calibration) into the accel/brake pedal calibration data, falling
 *          back to defaults where the persisted values are invalid. Call this after a successful
 *          calibration to make the new values take effect without a reboot.
 * @param   storage Pointer to the front controller storage
 * @return  STATUS_CODE_OK if the calibration data is applied successfully
 *          STATUS_CODE_INVALID_ARGS if storage is NULL
 */
StatusCode pedal_calib_apply(FrontControllerStorage *storage);

/**
 * @brief   Reads pedal calibration data from flash and applies it
 * @details Loads the persisted blob into storage->pedal_persist_data then calls pedal_calib_apply
 * @param   storage Pointer to the front controller storage
 * @return  STATUS_CODE_OK if calibration data is read successfully
 *          STATUS_CODE_INVALID_ARGS if storage is NULL
 */
StatusCode pedal_calib_read(FrontControllerStorage *storage);

/** @} */
