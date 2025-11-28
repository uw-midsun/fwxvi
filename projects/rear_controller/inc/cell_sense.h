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
#include "rear_controller_getters.h"
#include "rear_controller_setters.h"

/**
 * @defgroup bms_carrier
 * @brief    bms_carrier Firmware
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

StatusCode cell_sense_init(RearControllerStorage *storage);

// Mark cell for discharging (takes effect after config is re-written)
// |cell| should be [0, settings.num_cells)

StatusCode cell_discharge(AdbmsAfeStorage *afe);

/** @} */
