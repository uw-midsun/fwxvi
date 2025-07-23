/************************************************************************************************
 * @file   pedal_calib.c
 *
 * @brief  Source file to implement Pedal Calibration
 *
 * @date   2025-05-15
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdio.h>
#include <string.h>

/* Inter-component Headers */
#include "pedal_calib.h"

/* Intra-component Headers */
#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupts.h"
#include "log.h"

// Pedal Calibration function
StatusCode pedal_calib_sample(PedalCalibrationStorage *calib_storage, PedalCalibrationData *data, PedalState state, GpioAddress *address) {
  // Erase existing data at storage location
  memset(calib_storage, 0, sizeof(*calib_storage));

  // Reset variables for pedal calibration storage
  int32_t average_value = 0;
  calib_storage->sample_counter = 0;
  calib_storage->min_reading = INT16_MAX;
  calib_storage->max_reading = INT16_MIN;

  StatusCode status;
  while (calib_storage->sample_counter < NUM_SAMPLES) {
    // Read the values from the MAX, at this point the pedal should be in either
    // a fully pressed or released state
    uint16_t adc_reading;
    status = adc_read_raw(address, &adc_reading);
    if (status != STATUS_CODE_OK) {
      return STATUS_CODE_INCOMPLETE;
    }
    uint16_t reading = (uint16_t)adc_reading;
    calib_storage->sample_counter++;
    average_value += reading;
    if (calib_storage->min_reading > reading) {
      calib_storage->min_reading = reading;
    }

    if (calib_storage->max_reading < reading) {
      calib_storage->max_reading = reading;
    }
  }

  if (state == PEDAL_PRESSED) {
    data->upper_value = average_value / NUM_SAMPLES;
  } else if (state == PEDAL_UNPRESSED) {
    data->lower_value = average_value / NUM_SAMPLES;
  } else {
    return STATUS_CODE_INVALID_ARGS;
  }
  return STATUS_CODE_OK;
}
