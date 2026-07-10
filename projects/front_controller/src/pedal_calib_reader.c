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
#include "global_enums.h"
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

StatusCode pedal_calib_apply(FrontControllerStorage *storage) {
  if (storage == NULL || storage->pedal_persist_data == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  PedalPersistData *persist_data = storage->pedal_persist_data;

  // Set the amplified lower and upper values
  if (persist_data->accel_pedal_data_amplified.lower_value == persist_data->accel_pedal_data_amplified.upper_value) {
    storage->accel_pedal_storage->calibration_data.lower_value = FRONT_CONTROLLER_ACCEL_PEDAL_LOWER_VALUE_AMPLIFIED_DEFAULT;
    storage->accel_pedal_storage->calibration_data.upper_value = FRONT_CONTROLLER_ACCEL_PEDAL_UPPER_VALUE_AMPLIFIED_DEFAULT;
    storage->accel_pedal_storage->calibration_data.reversed = FRONT_CONTROLLER_ACCEL_PEDAL_REVERSED_DEFAULT;
  } else if (persist_data->accel_pedal_data_amplified.lower_value < persist_data->accel_pedal_data_amplified.upper_value) {
    storage->accel_pedal_storage->calibration_data.lower_value = persist_data->accel_pedal_data_amplified.lower_value;
    storage->accel_pedal_storage->calibration_data.upper_value = persist_data->accel_pedal_data_amplified.upper_value;
    storage->accel_pedal_storage->calibration_data.reversed = false;
  } else {
    storage->accel_pedal_storage->calibration_data.lower_value = persist_data->accel_pedal_data_amplified.upper_value;
    storage->accel_pedal_storage->calibration_data.upper_value = persist_data->accel_pedal_data_amplified.lower_value;
    storage->accel_pedal_storage->calibration_data.reversed = true;
  }

  // Set the opamp offset
  if (persist_data->accel_pedal_data_raw.lower_value == persist_data->accel_pedal_data_raw.upper_value) {
    storage->accel_pedal_storage->calibration_data.opamp_offset = FRONT_CONTROLLER_ACCEL_PEDAL_OPAMP_THRESHOLD_DEFAULT;
  }
  if (persist_data->accel_pedal_data_raw.lower_value < persist_data->accel_pedal_data_raw.upper_value) {
    storage->accel_pedal_storage->calibration_data.opamp_offset = persist_data->accel_pedal_data_raw.lower_value;
  } else {
    storage->accel_pedal_storage->calibration_data.opamp_offset = persist_data->accel_pedal_data_raw.upper_value;
  }

  // Set the brake pedal upper and lower values
  if (persist_data->brake_pedal_data.lower_value == persist_data->brake_pedal_data.upper_value) {
    storage->brake_pedal_storage->calibration_data.lower_value = FRONT_CONTROLLER_BRAKE_PEDAL_LOWER_VALUE_DEFAULT;
    storage->brake_pedal_storage->calibration_data.upper_value = FRONT_CONTROLLER_BRAKE_PEDAL_UPPER_VALUE_DEFAULT;
  } else {
    storage->brake_pedal_storage->calibration_data.lower_value = persist_data->brake_pedal_data.lower_value;
    storage->brake_pedal_storage->calibration_data.upper_value = persist_data->brake_pedal_data.upper_value;
  }

  return STATUS_CODE_OK;
}

StatusCode pedal_calib_read(FrontControllerStorage *storage) {
  if (storage == NULL || storage->persist_storage == NULL || storage->pedal_persist_data == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Load the persisted calibration from flash into the shared pedal persist data */
  status_ok_or_return(persist_init(storage->persist_storage, flash_app_storage_page(), storage->pedal_persist_data, sizeof(*storage->pedal_persist_data), false));

  /* Apply the loaded values (or defaults) to the pedal storage */
  return pedal_calib_apply(storage);
}
