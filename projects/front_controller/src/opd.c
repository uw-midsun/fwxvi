/************************************************************************************************
 * @file   opd.c
 *
 * @brief  Source file for one pedal drive manager
 *
 * @date   2025-09-14
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
#include "front_controller_hw_defs.h"
#include "opd.h"

static GpioAddress s_one_pedal_gpio = FRONT_CONTROLLER_ACCEL_PEDAL;
static FrontControllerStorage *front_controller_storage;

static OpdStorage s_one_pedal_storage = { 0U };
static bool pts_compare_handler(float p, float s, PtsRelationType relation_type);

StatusCode opd_run() {
  if (front_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  uint16_t adc_reading = s_one_pedal_storage.calibration_data.lower_value;

  adc_read_raw(&s_one_pedal_gpio, &adc_reading);

  float normalized_accel_percentage = (((float)adc_reading - (float)s_one_pedal_storage.calibration_data.lower_value) /
                                       ((float)s_one_pedal_storage.calibration_data.upper_value - (float)s_one_pedal_storage.calibration_data.lower_value));

  normalized_accel_percentage = fminf(fmaxf(normalized_accel_percentage, 0.0f), 1.0f);

  float calculated_reading;
  if (normalized_accel_percentage < front_controller_storage->config->accel_input_deadzone) {
    calculated_reading = 0.0f;
  } else {
    opd_linear_calculate(normalized_accel_percentage, PTS_TYPE_LINEAR, &calculated_reading);
  }

  s_one_pedal_storage.accel_percentage = calculated_reading;

  return STATUS_CODE_OK;
}

StatusCode opd_init(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  front_controller_storage = storage;
  front_controller_storage->opd_storage = &s_one_pedal_storage;

  gpio_init_pin(&s_one_pedal_gpio, GPIO_ANALOG, GPIO_STATE_LOW);
  adc_add_channel(&s_one_pedal_gpio);

  return STATUS_CODE_OK;
}

StatusCode opd_linear_calculate(float pedal_percentage, PtsRelationType relation_type, float *calculated_reading) {
  float current_speed = (float)((float)front_controller_storage->vehicle_speed_kph / (float)s_one_pedal_storage.max_vehicle_speed_kph);

  if (pts_compare_handler(pedal_percentage, current_speed, relation_type)) {
    front_controller_storage->brake_enabled = false;
    s_one_pedal_storage.accel_state = ACCEL_STATE_DRIVING;
    float m = 1 / (1 - current_speed);
    *calculated_reading = (0.25 * m * (pedal_percentage - 1)) + 1;
  } else {
    front_controller_storage->brake_enabled = true;
    s_one_pedal_storage.accel_state = ACCEL_STATE_BRAKING;
    float m = 1 / current_speed;
    *calculated_reading = s_one_pedal_storage.max_braking_percentage * (1 - (m * pedal_percentage));
  }

  return STATUS_CODE_OK;
}

StatusCode opd_quadratic_calculate(float pedal_percentage, PtsRelationType relation_type, float *calculated_reading) {
  float current_speed = (float)((float)front_controller_storage->vehicle_speed_kph / (float)s_one_pedal_storage.max_vehicle_speed_kph);
  float m;
  if (pts_compare_handler(pedal_percentage, current_speed, relation_type)) {
    front_controller_storage->brake_enabled = false;
    s_one_pedal_storage.accel_state = ACCEL_STATE_DRIVING;
    m = 1 / ((1 - current_speed) * (1 - current_speed));
  } else {
    front_controller_storage->brake_enabled = true;
    s_one_pedal_storage.accel_state = ACCEL_STATE_BRAKING;
    m = s_one_pedal_storage.max_braking_percentage / (current_speed * current_speed);
  }
  *calculated_reading = m * (pedal_percentage - current_speed) * (pedal_percentage - current_speed);

  return STATUS_CODE_OK;
}

StatusCode opd_calculate_handler(float pedal_percentage, PtsRelationType relation_type, float *calculated_reading, curveType curve_type) {
  switch (curve_type) {
    case CURVE_TYPE_LINEAR:
      return opd_linear_calculate(pedal_percentage, relation_type, calculated_reading);
    case CURVE_TYPE_QUADRATIC:
      return opd_quadratic_calculate(pedal_percentage, relation_type, calculated_reading);
    case CURVE_TYPE_EXPONENTIAL: {
      StatusCode ret = opd_linear_calculate(pedal_percentage, relation_type, calculated_reading);
      *calculated_reading = powf(*calculated_reading, front_controller_storage->config->accel_input_curve_exponent);
      return ret;
    }
  }

  return STATUS_CODE_OK;
}

static bool pts_compare_handler(float p, float s, PtsRelationType relation_type) {
  switch (relation_type) {
    case PTS_TYPE_LINEAR:
      if (p > s) {
        return true;
      } else {
        return false;
      }
    case PTS_TYPE_EXPONENTIAL:
      if (powf(p, 2) > s) {
        return true;
      } else {
        return false;
      }
    case PTS_TYPE_QUADRATIC:
      if (p * p > s) {
        return true;
      } else {
        return false;
      }
    default:
      return false;
  }

  // should never get here
  return false;
}