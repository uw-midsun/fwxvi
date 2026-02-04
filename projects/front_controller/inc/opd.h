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

typedef struct OpdStorage {
  float max_braking_percentage;   /**< Max OPD regen braking percentage */
  uint32_t max_vehicle_speed_kph; /**< Max vehicle speed in KPH */
} OpdStorage;

typedef enum PtsRelationType { PTS_TYPE_LINEAR, PTS_TYPE_EXPONENTIAL, PTS_TYPE_QUADRATIC } PtsRelationType;

typedef enum CurveType { CURVE_TYPE_LINEAR, CURVE_TYPE_EXPONENTIAL, CURVE_TYPE_QUADRATIC } CurveType;

/**
 * @brief handler for opd_calculate command, runs either linear or quadratic calculation based on CurveType enum
 */
StatusCode opd_calculate_handler(float pedal_percentage, PtsRelationType relation_type, float *calculated_reading, CurveType curve_type);

/**
 * @brief Perform linear OPD calculation
 */
StatusCode opd_linear_calculate(float pedal_percentage, PtsRelationType relation_type, float *calculated_reading);

/**
 * @brief Perform quadratic OPD calculation
 */
StatusCode opd_quadratic_calculate(float pedal_percentage, PtsRelationType relation_type, float *calculated_reading);

/**
 * @brief   Reads the pedal percentage from AccelPedalStorage and saves an adjusted acceleration percentage to front controller based on OPD algorithm
 * @return  STATUS_CODE_OK if acceleration percentage is successfully stored
 *          STATUS_CODE_UNINITIALIZED if front controller storage is uninitialized
 */
StatusCode opd_run();

/**
 * @brief   Initializes the opd, loads front controller storage
 *
 * @return  STATUS_CODE_OK if opd is initialized successfully
 *          STATUS_CODE_INVALID_ARGS if storage pointer is invalid
 */
StatusCode opd_init(FrontControllerStorage *storage);

/** @} */
