#pragma once

/************************************************************************************************
 * @file   cell_sense.h
 *
 * @brief  Header file for AFE cell sense
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* Inter-component Headers */
#include "gpio.h"

/* Intra-component Headers */
#include "rear_controller.h"

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

#define CONV_DELAY_MS 10
#define AUX_CONV_DELAY_MS 3
// Maximum number of retry attempts to read cell/aux data once triggered
#define RETRY_DELAY_MS 1
#define CELL_SENSE_CONVERSIONS 0

#define SOLAR_VOLTAGE_THRESHOLD 42000
#define AFE_BALANCING_UPPER_THRESHOLD 41500
#define AFE_BALANCING_LOWER_THRESHOLD 40000

/**
 * @brief   Initializes the cell sense sub-system
 * @param   storage Pointer to the rear controller storage
 * @return  STATUS_CODE_OK if initialized succesfully
 *          STATUS_CODE_INVALID_ARGS if invalid parameter is passed in
 */
StatusCode cell_sense_init(RearControllerStorage *storage);

/**
 * @brief   Logs the newest cell sense readings
 * @return  STATUS_CODE_OK if logged succesfully
 *          STATUS_CODE_UNINITIALIZED if system is not initialized
 */
StatusCode log_cell_sense();

/** @} */
