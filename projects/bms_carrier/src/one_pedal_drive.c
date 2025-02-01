/************************************************************************************************
 * @file   one_pedal_drive.c
 *
 * @brief  Source file for One pedal drive
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <math.h>

/* Inter-component Headers */
#include "log.h"

/* Intra-component Headers */
#include "one_pedal_drive.h"
#include "bms_carrier_setters.h"

float opd_threshold(float car_velocity_kmh) {
  float threshold = 0.0f;
  if (fabs(car_velocity_kmh) <= MAX_OPD_SPEED) {
    threshold = fabs(car_velocity_kmh) * COASTING_THRESHOLD_SCALE;
  } else {
    threshold = MAX_COASTING_THRESHOLD;
  }
  return threshold;
}

float opd_current(float throttle_percent, float threshold, DriveState *drive_state) {
  /* Handle negative inputs */
  if (throttle_percent < 0.0f || threshold < 0.0f) {
    *drive_state = BRAKE;
    return 0.0f;
  }

  /* Handle buffer region around the dynamic threshold */
  if (throttle_percent <= threshold + 0.05 && throttle_percent >= threshold - 0.05) {
    return 0.0f;
  }

  if (throttle_percent >= threshold) {
    /* If greater than the dynamic threshold, accelerate */
    set_motor_velocity_brakes_enabled(false);
    return (throttle_percent - threshold) / (1.0f - threshold);
  } else {
    /* If less than the dynamic threshold, regen brake */
    *drive_state = BRAKE;

    set_motor_velocity_brakes_enabled(true);
    return (threshold - throttle_percent) / (threshold);
  }

  /* Should not reach this */
  LOG_DEBUG("ERROR: One pedal throttle not calculated\n");
  return 0.0f;
}
