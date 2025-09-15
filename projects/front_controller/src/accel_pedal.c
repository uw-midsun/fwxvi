/************************************************************************************************
 * @file   accel_pedal.c
 *
 * @brief  Source file for acceleration pedal manager
 *
 * @date   2025-01-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <math.h>

/* Inter-component Headers */
#include "adc.h"
#include "gpio.h"
#include "log.h"
#include "pedal_calib.h"

/* Intra-component Headers */
#include "accel_pedal.h"
#include "front_controller_hw_defs.h"

static GpioAddress s_accel_pedal_gpio = FRONT_CONTROLLER_ACCEL_PEDAL;

static FrontControllerStorage *front_controller_storage;

static AccelPedalStorage s_accel_pedal_storage = { 0U };

StatusCode accel_pedal_run() {
  if (front_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  uint16_t adc_reading = s_accel_pedal_storage.calibration_data.lower_value;
  adc_read_raw(&s_accel_pedal_gpio, &adc_reading);

  /**
   * Convert ADC Reading to readable voltage by normalizing with calibration data and dividing
   * to get percentage press. Negatives and > 100 values will be clamped
   */
  float calculated_reading = (((float)adc_reading - (float)s_accel_pedal_storage.calibration_data.lower_value) /
                              ((float)s_accel_pedal_storage.calibration_data.upper_value - (float)s_accel_pedal_storage.calibration_data.lower_value));

  calculated_reading = fminf(fmaxf(calculated_reading, 0.0f), 1.0f);

  /* Check the reading is above the input deadzone */
  if (calculated_reading < front_controller_storage->config->accel_input_deadzone) {
    calculated_reading = 0.0f;
  } else {
    /* Exponential curve shaping */
    calculated_reading = powf(calculated_reading, front_controller_storage->config->accel_input_curve_exponent);

    /* Map the [0.0 - 1.0] reading to [remap min - 1.0] */
    calculated_reading = front_controller_storage->config->accel_input_remap_min + ((1.0f - front_controller_storage->config->accel_input_remap_min) * calculated_reading);
  }

  s_accel_pedal_storage.accel_percentage = front_controller_storage->config->accel_low_pass_filter_alpha * calculated_reading +
                                           (1.0f - front_controller_storage->config->accel_low_pass_filter_alpha) * s_accel_pedal_storage.prev_accel_percentage;
  s_accel_pedal_storage.prev_accel_percentage = calculated_reading;

  return STATUS_CODE_OK;
}

StatusCode accel_pedal_init(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  front_controller_storage = storage;
  front_controller_storage->accel_pedal_storage = &s_accel_pedal_storage;

  /* Initialize hardware */
  gpio_init_pin(&s_accel_pedal_gpio, GPIO_ANALOG, GPIO_STATE_LOW);
  adc_add_channel(&s_accel_pedal_gpio);

  // TODO: calib_init(&s_accel_pedal_storage.calibration_data, sizeof(s_accel_pedal_storage.calibration_data), false);
  return STATUS_CODE_OK;
}
