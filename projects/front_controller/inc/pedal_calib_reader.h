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
 * @brief   Reads pedal calibration data
 * @param   storage Pointer to the front controller storage
 * @return  STATUS_CODE_OK if calibration data is read successfully
 *          STATUS_CODE_INVALID_ARGS if storage is NULL
 */
StatusCode pedal_calib_read(FrontControllerStorage *storage);

/** @} */
