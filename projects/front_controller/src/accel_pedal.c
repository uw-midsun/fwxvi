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
#include "dac.h"
#include "gpio.h"
#include "log.h"
#include "opamp.h"
#include "pedal_calib.h"

/* Intra-component Headers */
#include "accel_pedal.h"
#include "front_controller_hw_defs.h"

static GpioAddress s_accel_pedal_gpio_raw = GPIO_FRONT_CONTROLLER_ACCEL_PEDAL_RAW;
static GpioAddress s_accel_pedal_gpio_opamp_out = GPIO_FRONT_CONTROLLER_ACCEL_PEDAL_OPAMP_OUT;

static FrontControllerStorage *front_controller_storage;

static AccelPedalStorage s_accel_pedal_storage = { 0U };

#define ACCEL_PEDAL_DEBUG 0U

#if (ACCEL_PEDAL_DEBUG == 1)
#define CONDITIONAL_LOG_DEBUG(...) LOG_DEBUG(__VA_ARGS__)
#else
#define CONDITIONAL_LOG_DEBUG(...) \
  do {                             \
  } while (0)
#endif

StatusCode accel_pedal_run() {
  if (front_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  uint16_t adc_reading;
  adc_read_raw(&s_accel_pedal_gpio_opamp_out, &adc_reading);

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

  if (s_accel_pedal_storage.calibration_data.reversed) {
    calculated_reading = 1.0f - calculated_reading;
  }

  // Square the value
  calculated_reading = calculated_reading * calculated_reading;
  calculated_reading -= 0.08;

  if (calculated_reading < 0) {
    calculated_reading = 0.0f;
  }

  s_accel_pedal_storage.accel_percentage = front_controller_storage->config->accel_low_pass_filter_alpha * calculated_reading +
                                           (1.0f - front_controller_storage->config->accel_low_pass_filter_alpha) * s_accel_pedal_storage.prev_accel_percentage;

  if (s_accel_pedal_storage.accel_percentage <= 0.05) {
    s_accel_pedal_storage.accel_percentage = 0;
  }
  s_accel_pedal_storage.prev_accel_percentage = calculated_reading;

  front_controller_storage->accel_percentage = s_accel_pedal_storage.accel_percentage;

  CONDITIONAL_LOG_DEBUG("ACCEL READING: %d | CALC READING %ld \r\n", adc_reading, (int32_t)(s_accel_pedal_storage.accel_percentage * 100));

  return STATUS_CODE_OK;
}

StatusCode accel_pedal_init(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  front_controller_storage = storage;
  front_controller_storage->accel_pedal_storage = &s_accel_pedal_storage;

  return STATUS_CODE_OK;
}

StatusCode accel_pedal_start() {
  /* Initialize hardware */
  adc_add_channel(&s_accel_pedal_gpio_opamp_out);

  dac_enable_channel(FRONT_CONTROLLER_ACCEL_PEDAL_OPAMP_VREF_DAC);
  dac_set_voltage(FRONT_CONTROLLER_ACCEL_PEDAL_OPAMP_VREF_DAC, s_accel_pedal_storage.calibration_data.opamp_offset);

  OpampConfig config = {
    .vinp_sel = OPAMP_NONINVERTING_IO0, /* PA1 - Pedal input */
    .vinm_sel = OPAMP_INVERTING_IO0,    /* PA0 - External feedback network */
    .output_to_adc = true               /* Flag for documentation */
  };

  opamp_configure(FRONT_CONTROLLER_ACCEL_PEDAL_OPAMP, &config);
  opamp_start(FRONT_CONTROLLER_ACCEL_PEDAL_OPAMP);

  return STATUS_CODE_OK;
}
