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
#include "steering_getters.h"

#define MIN_CELL_VOLTAGE 2500.0f
#define VOLTAGE_DROP_PER_LAP 24.02f
#define DISTANCE_PER_LAP_KM 5.07f //track is 3.15miles which is 5.07km

float estimate_remaining_range_km(void) {
    float cell_voltage = get_battery_stats_B_min_cell_voltage();

    if (cell_voltage < MIN_CELL_VOLTAGE) {
        return 0.0f;
    }
    float voltage_remaining = cell_voltage - MIN_CELL_VOLTAGE; //calculate remaining voltage

    float laps_remaining = voltage_remaining / VOLTAGE_DROP_PER_LAP; //calculate remaining laps

    float km_remaining = laps_remaining * DISTANCE_PER_LAP_KM; //calculate remaining km

    printf("Remaining range: %.2f km\n", km_remaining); //for debugging

    return km_remaining;
}