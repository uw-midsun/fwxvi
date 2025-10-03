#pragma once

/************************************************************************************************
 * @file   opd.h
 *
 * @brief  Header file for one pedal drive manager
 *
 * @date   2025-09-14
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "front_controller.h"

/**
 * @defgroup Front_Controller
 * @brief    Front Controller Board Firmware
 * @{
 */

typedef struct {
  uint16_t lower_value; /**< ADC reading when the pedal is considered fully released */
  uint16_t upper_value; /**< ADC reading when the pedal is considered fully pressed */
} OpdCalibrationStorage;

typedef enum { ACCEL_STATE_DRIVING, ACCEL_STATE_BRAKING } AccelState;

typedef struct OpdStorage {
  float accel_percentage;
  float prev_accel_percentage;
  AccelState accel_state;
  float max_braking_percentage;
  uint32_t max_vehicle_speed_kph;

  OpdCalibrationStorage calibration_data;
} OpdStorage;

typedef enum { PTS_TYPE_LINEAR, PTS_TYPE_EXPONENTIAL, PTS_TYPE_QUADRATIC } PtsRelationType;

typedef enum { CURVE_TYPE_LINEAR, CURVE_TYPE_EXPONENTIAL, CURVE_TYPE_QUADRATIC } curveType;

StatusCode opd_run();

StatusCode opd_init(FrontControllerStorage *storage);

StatusCode opd_calculate_handler(float pedal_percentage, PtsRelationType relation_type, float *calculated_reading, curveType curve_type);

StatusCode opd_linear_calculate(float pedal_percentage, PtsRelationType relation_type, float *calculated_reading);

StatusCode opd_quadratic_calculate(float pedal_percentage, PtsRelationType relation_type, float *calculated_reading);
