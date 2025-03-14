#pragma once

/************************************************************************************************
 * @file   pedal.h
 *
 * @brief  Header file for pedal in centre_console
 *
 * @date   2025-02-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Inter-component Headers */
#include "cc_hw_defs.h"
#include "interrupts.h"
#include "log.h"
#include "status.h"

/* Intra-component Headers */

/**
 * @defgroup pedal
 * @brief    pedal for centre_console Firmware
 * @{
 */

typedef struct PedalCalibrationData {
  // When the pedal is considered fully unpressed
  int16_t lower_value;
  // When the pedal is considered fully pressed
  int16_t upper_value;
} PedalCalibrationData;

typedef struct PedalCalibBlob {
  PedalCalibrationData throttle_calib;
  PedalCalibrationData brake_calib;
} PedalCalibBlob;

// Runs a pedal cycle to update throttle and brake values
void pedal_run();

// Initializes pedal peripherals
StatusCode pedal_init(PedalCalibBlob *calib_blob);

/** @} */
