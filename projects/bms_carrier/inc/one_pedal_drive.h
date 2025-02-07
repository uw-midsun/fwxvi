#pragma once

/************************************************************************************************
 * @file   one_pedal_drive.h
 *
 * @brief  Source code for One pedal drive
 *
 * @date   2025-02-06
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "global_enums.h"

/* Intra-component Headers */
#include "bms_carrier.h"

/**
 * @defgroup motor_controller
 * @brief    motor_controller Firmware
 * @{
 */

/** @brief  Max pedal threshold when coasting at speeds > 8 km/h */
#define MAX_COASTING_THRESHOLD 0.4f
/** @brief  Max car speed in km/h before one pedal drive threshold maxes out */
#define MAX_OPD_SPEED 16.0f
/** @brief  Scaling value to determine the coasting threshold based on speed */
#define COASTING_THRESHOLD_SCALE 0.025f

/**
 * @brief   One pedal drive storage
 */
struct OnePedalStorage {
    float opd_threshold;        /**< Aux sense analog pin */
};

/**
 * @brief   Calclulate the one pedal drive threshold
 * @param   car_velocity_kmh Vehicle velocity in kilometers per hour
 * @return  Threshold value between 1.0 and 0.0
 */
float opd_threshold(float car_velocity_kmh);

/**
 * @brief   Calculate the motor current based on the pedal position, threshold and drive state
 * @return  Motor current value between 1.0 and 0.0
 */
float opd_current(float throttle_percent, float threshold, DriveState *drive_state);

/** @} */
