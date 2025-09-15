/************************************************************************************************
 * @file    brake_pedal.c
 *
 * @brief   Source file for brake pedal manager
 *
 * @date    2025-09-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <math.h>

/* Inter-component Headers */
#include "adc.h"
#include "gpio.h"
#include "log.h"
#include "pedal_calib.h"

/* Intra-component Headers */
#include "brake_pedal.h"
#include "front_controller_hw_defs.h"

static GpioAddress s_brake_gpio = FRONT_CONTROLLER_BRAKE_PEDAL;
static FrontControllerStorage *front_controller_storage = NULL;
static BrakePedalStorage s_brake_pedal_storage = { 0U };

StatusCode brake_pedal_run() {
  if (front_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  uint16_t adc_reading = s_brake_pedal_storage.calibration_data.lower_value;
  adc_read_raw(&s_brake_gpio, &adc_reading);

  float calculated_reading = ((float)adc_reading - (float)s_brake_pedal_storage.calibration_data.lower_value) /
                             ((float)s_brake_pedal_storage.calibration_data.upper_value - (float)s_brake_pedal_storage.calibration_data.lower_value);

  calculated_reading =
      calculated_reading * front_controller_storage->config->brake_low_pass_filter_alpha + (1.0f - front_controller_storage->config->brake_low_pass_filter_alpha) * s_brake_pedal_storage.prev_reading;
  s_brake_pedal_storage.prev_reading = calculated_reading;

  if (calculated_reading > front_controller_storage->config->brake_pedal_deadzone) {
    front_controller_storage->brake_enabled = true;
  } else {
    front_controller_storage->brake_enabled = false;
  }

  return STATUS_CODE_OK;
}

StatusCode brake_pedal_init(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  front_controller_storage = storage;
  front_controller_storage->brake_pedal_storage = &s_brake_pedal_storage;

  gpio_init_pin(&s_brake_gpio, GPIO_ANALOG, GPIO_STATE_LOW);
  adc_add_channel(&s_brake_gpio);
  return STATUS_CODE_OK;
}
