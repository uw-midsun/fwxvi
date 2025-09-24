/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for pedal_calib_reader
 *
 * @date   2025-09-16
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "accel_pedal.h"
#include "brake_pedal.h"
#include "pedal_calib.h"
#include "persist.h"

#define LAST_PAGE (NUM_FLASH_PAGES - 1)

typedef struct PedalPersistData {
  PedalCalibrationData accel_pedal_data;
  PedalCalibrationData brake_pedal_data;
} PedalPersistData;

StatusCode pedal_calib_read(FrontControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  PersistStorage persist_storage;
  PedalPersistData persist_data = { 0U };

  persist_init(&persist_storage, LAST_PAGE, &persist_data, sizeof(persist_data), false);

  storage->accel_pedal_storage->calibration_data.lower_value = persist_data.accel_pedal_data.lower_value;
  storage->accel_pedal_storage->calibration_data.upper_value = persist_data.accel_pedal_data.upper_value;

  storage->brake_pedal_storage->calibration_data.lower_value = persist_data.brake_pedal_data.lower_value;
  storage->brake_pedal_storage->calibration_data.upper_value = persist_data.brake_pedal_data.upper_value;

  return STATUS_CODE_OK;
}
