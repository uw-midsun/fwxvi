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
#include "accel_pedal.h"
#include "front_controller_getters.h"
#include "front_controller_hw_defs.h"
#include "opd.h"

#define REGEN_BRAKING_VOLTAGE_RAMP_OFFSET 50.0f
#define MAX_CELL_VOLTAGE 4200.0f

static FrontControllerStorage *front_controller_storage;
static AccelPedalStorage *accel_pedal_storage;

static OpdStorage s_one_pedal_storage = { 0U };
static bool pts_compare_handler(float p, float s, PtsRelationType relation_type);

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

// Linearly reduce regenerative braking when it's within a certain delta of the max cell voltage
void opd_limit_regen_when_charged(float *calculated_reading) {
  if (!front_controller_storage->brake_enabled) return;

  float cell_voltage = get_battery_stats_B_min_cell_voltage();
  float scaler = 1.0;
  if (cell_voltage >= MAX_CELL_VOLTAGE) {
    scaler = 0.0;
  } else if (cell_voltage >= MAX_CELL_VOLTAGE - REGEN_BRAKING_VOLTAGE_RAMP_OFFSET) {
    scaler = (MAX_CELL_VOLTAGE - cell_voltage) / REGEN_BRAKING_VOLTAGE_RAMP_OFFSET;
  }

  *calculated_reading *= scaler;
}

StatusCode opd_linear_calculate(float pedal_percentage, PtsRelationType relation_type, float *calculated_reading) {
  float current_speed = (float)((float)front_controller_storage->vehicle_speed_kph / (float)s_one_pedal_storage.max_vehicle_speed_kph);

  if (pts_compare_handler(pedal_percentage, current_speed, relation_type)) {
    front_controller_storage->brake_enabled = false;
    s_one_pedal_storage.drive_state = STATE_DRIVING;
    float m = 1 / (1 - current_speed);
    *calculated_reading = (0.25 * m * (pedal_percentage - 1)) + 1;
  } else {
    front_controller_storage->brake_enabled = true;
    s_one_pedal_storage.drive_state = STATE_BRAKING;
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
    s_one_pedal_storage.drive_state = STATE_DRIVING;
    m = 1 / ((1 - current_speed) * (1 - current_speed));
  } else {
    front_controller_storage->brake_enabled = true;
    s_one_pedal_storage.drive_state = STATE_BRAKING;
    m = s_one_pedal_storage.max_braking_percentage / (current_speed * current_speed);
  }
  *calculated_reading = m * (pedal_percentage - current_speed) * (pedal_percentage - current_speed);

  return STATUS_CODE_OK;
}

StatusCode opd_calculate_handler(float pedal_percentage, PtsRelationType relation_type, float *calculated_reading, CurveType curve_type) {
  StatusCode ret;
  switch (curve_type) {
    case CURVE_TYPE_LINEAR:
      ret = opd_linear_calculate(pedal_percentage, relation_type, calculated_reading);
    case CURVE_TYPE_QUADRATIC:
      ret = opd_quadratic_calculate(pedal_percentage, relation_type, calculated_reading);
    case CURVE_TYPE_EXPONENTIAL: {
      ret = opd_linear_calculate(pedal_percentage, relation_type, calculated_reading);
      *calculated_reading = powf(*calculated_reading, front_controller_storage->config->accel_input_curve_exponent);
    }
    default: {
      return STATUS_CODE_OK;
    }
  }

  opd_limit_regen_when_charged(calculated_reading);
  return ret;
}

StatusCode opd_run() {
  if (front_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  float accel_percentage = accel_pedal_storage->accel_percentage;

  float calculated_reading = 0;

  StatusCode ret = opd_linear_calculate(accel_percentage, PTS_TYPE_LINEAR, &calculated_reading);
  if (ret != STATUS_CODE_OK) {
    return ret;
  }

  front_controller_storage->accel_percentage = calculated_reading;

  return STATUS_CODE_OK;
}

StatusCode opd_init(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  front_controller_storage = storage;
  front_controller_storage->opd_storage = &s_one_pedal_storage;

  accel_pedal_storage = storage->accel_pedal_storage;
  return STATUS_CODE_OK;
}
