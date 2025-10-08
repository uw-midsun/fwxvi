/************************************************************************************************
 * @file   range_estimator.c
 *
 * @brief  Source file for estimating driving range based on remaining voltage
 *
 * @date   2025-10-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "range_estimator.h"

#define MIN_CELL_VOLTAGE 2500.00
#define VOLTAGE_DROP_PER_LAP 24.02
#define DISTANCE_PER_LAP_KM 5.07 //track is 3.15miles which is 5.07km

float estimate_remaining_range_km(float read_voltage) {
    if (read_voltage < MIN_CELL_VOLTAGE) {
        return 0.0;
    }
    float voltage_remaining = read_voltage - MIN_CELL_VOLTAGE; //calculate remaining voltage

    float laps_remaining = voltage_remaining / VOLTAGE_DROP_PER_LAP; //calculate remaining laps

    float km_remaining = laps_remaining * DISTANCE_PER_LAP_KM; //calculate remaining km

    return km_remaining;
}