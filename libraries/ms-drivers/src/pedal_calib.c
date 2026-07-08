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
#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupts.h"
#include "log.h"

/* Intra-component Headers */
#include "pedal_calib.h"

static int32_t single_sample_average;
static StatusCode single_sample_ret;

// Pedal Calibration function
StatusCode pedal_calib_sample(PedalCalibrationStorage *calib_storage, PedalCalibrationData *data, PedalState state, GpioAddress *address) {
  // Erase existing data at storage location
  memset(calib_storage, 0, sizeof(*calib_storage));
  delay_ms(1000U);
  LOG_DEBUG("PEDAL_CALIB_SEQUENCE\r\n");

  // Reset variables for pedal calibration storage
  int32_t average_value = 0;
  calib_storage->sample_counter = 0;
  calib_storage->min_reading = INT16_MAX;
  calib_storage->max_reading = INT16_MIN;

  StatusCode status;
  while (calib_storage->sample_counter < NUM_SAMPLES_ONE_SHOT) {
    adc_run();

    // Read the values from the MAX, at this point the pedal should be in either
    // a fully pressed or released state
    uint16_t adc_reading;
    status = adc_read_raw(address, &adc_reading);

    if (adc_reading <= 10) {
      LOG_DEBUG("Val too small, skipping: %u\r\n", adc_reading);
      return STATUS_CODE_OK;
    }

    if (status != STATUS_CODE_OK) {
      return STATUS_CODE_INCOMPLETE;
    }
    calib_storage->sample_counter++;
    average_value += adc_reading;
    if (calib_storage->min_reading > adc_reading) {
      calib_storage->min_reading = adc_reading;
    }

    if (calib_storage->max_reading < adc_reading) {
      calib_storage->max_reading = adc_reading;
    }

    LOG_DEBUG("Sampling %u: %u\r\n", (uint16_t)calib_storage->sample_counter, adc_reading);

    delay_ms(1U);
  }

  if (state == PEDAL_PRESSED) {
    data->upper_value = average_value / NUM_SAMPLES_ONE_SHOT;
  } else if (state == PEDAL_UNPRESSED) {
    data->lower_value = average_value / NUM_SAMPLES_ONE_SHOT;
  } else {
    return STATUS_CODE_INVALID_ARGS;
  }
  return STATUS_CODE_OK;
}

StatusCode pedal_calib_sample_single(PedalCalibrationStorage *calib_storage, PedalCalibrationData *data, PedalState state, GpioAddress *address, bool first_sample) {
  if (first_sample) {
    memset(calib_storage, 0, sizeof(*calib_storage));
    single_sample_average = 0;
    calib_storage->sample_counter = 0;
    calib_storage->min_reading = INT16_MAX;
    calib_storage->max_reading = INT16_MIN;
  }

  if (calib_storage->sample_counter < NUM_SAMPLES_SINGLE_READ) {
    adc_run();
    uint16_t adc_reading;
    single_sample_ret = adc_read_raw(address, &adc_reading);

    if (single_sample_ret != STATUS_CODE_OK) {
      return STATUS_CODE_INCOMPLETE;
    }

    calib_storage->sample_counter++;
    single_sample_average += adc_reading;
    if (calib_storage->min_reading > adc_reading) {
      calib_storage->min_reading = adc_reading;
    }

    if (calib_storage->max_reading < adc_reading) {
      calib_storage->max_reading = adc_reading;
    }
    // LOG_DEBUG("Sampling %u: %u\r\n", (uint16_t)calib_storage->sample_counter, adc_reading);
  }

  if (calib_storage->sample_counter >= NUM_SAMPLES_SINGLE_READ) {
    if (state == PEDAL_PRESSED) {
      data->upper_value = single_sample_average / NUM_SAMPLES_SINGLE_READ;
    } else if (state == PEDAL_UNPRESSED) {
      data->lower_value = single_sample_average / NUM_SAMPLES_SINGLE_READ;
    } else {
      return STATUS_CODE_INVALID_ARGS;
    }

    return STATUS_CODE_OK;
  }

  return STATUS_CODE_INCOMPLETE;
}
