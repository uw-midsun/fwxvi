#pragma once

/************************************************************************************************
 * @file   pedal_calib.h
 *
 * @brief  Header file to define the pedal calibration data
 *
 * @date   2025-05-15
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "global_enums.h"
#include "gpio.h"

/* Intra-component Headers */

/**
 * @defgroup Pedal
 * @brief    Pedal library
 * @{
 */

/** @brief The number of data samples we should take when calibrating the pedal */
#define NUM_SAMPLES 1000

/**
 * @brief Stores the possible pedal states that the pedal could be in
 * @details This struct also has a NUM_PEDAL_STATES enum which stores how many possible pedal states there are
 */
typedef enum {
  PEDAL_PRESSED = 0,
  PEDAL_UNPRESSED,
  NUM_PEDAL_STATES,
} PedalState;

/** @brief Stores pedal calibration data for the break and the throttle pedals */
typedef struct PedalCalibBlob {
  PedalCalibrationData throttle_calib;
  PedalCalibrationData brake_calib;
} PedalCalibBlob;

/** @brief A struct that stores the max and min reading along with a sample counter for the data that is being passed in from ADC */
typedef struct PedalCalibrationStorage {
  uint16_t min_reading_raw;
  uint16_t min_reading_amplified;
  uint16_t max_reading_amplified;
  volatile uint32_t sample_counter;
} PedalCalibrationStorage;

/** @brief A global struct that stores the throttle pedal and calibration data */
extern PedalCalibBlob global_calib_blob;

/**
 * @brief Reads data from ADC and calculates appropriate pedal calibration values
 *
 * @param calib_storage - A struct for storing the max and min reading while looping through adc reading for a given number of samples
 * @param data - A struct that stores the final calculated values for the upper and lower values for the pedal
 * @param state - An enum that describes the current state that he pedal is in when calling the function
 * @param address - The GPIO address for the adc of the pedal
 * @return STATUS_CODE_OK on success
 */
StatusCode pedal_calib_sample(PedalCalibrationStorage *calib_storage, PedalCalibrationData *data, PedalState state, GpioAddress *address);
/** @} */
