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

/* Measured from FGSP 2025 */
#define VOLTAGE_DROP_PER_LAP 24.02f

/* Track is 3.15 miles which is 5.07 kilometers */
#define DISTANCE_PER_LAP_KM 5.07f

float estimate_remaining_range_km(void) {
  float cell_voltage = get_battery_stats_B_min_cell_voltage();

  if (cell_voltage < MIN_CELL_VOLTAGE) {
    return 0.0f;
  }

  /* Calculate remaining voltage */
  float voltage_remaining = cell_voltage - MIN_CELL_VOLTAGE;

  /* Calculate remaining laps */
  float laps_remaining = voltage_remaining / VOLTAGE_DROP_PER_LAP;

  /* Calclulate remaining kilometers */
  float km_remaining = laps_remaining * DISTANCE_PER_LAP_KM;

  return km_remaining;
}
