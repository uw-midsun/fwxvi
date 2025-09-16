/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for pedal_calib
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
#include "front_controller_hw_defs.h"
#include "pedal_calib.h"
#include "persist.h"

GpioAddress accel_pedal_gpio = { .port = GPIO_PORT_A, .pin = 3 };
GpioAddress brake_pedal_gpio = { .port = GPIO_PORT_A, .pin = 5 };

#define LAST_PAGE (NUM_FLASH_PAGES - 1)

typedef struct PedalPersistData {
  PedalCalibrationData accel_pedal_data;
  PedalCalibrationData brake_pedal_data;
} PedalPersistData;

TASK(pedal_calib, TASK_STACK_1024) {
  PedalPersistData pedal_persist_data = { 0U };
  PedalCalibrationStorage pedal_storage;

  PersistStorage persist_storage = { 0U };

  // accel pedal data collection & set to struct
  pedal_calib_sample(&pedal_storage, &(pedal_persist_data.accel_pedal_data), PEDAL_PRESSED, &accel_pedal_gpio);
  pedal_calib_sample(&pedal_storage, &(pedal_persist_data.accel_pedal_data), PEDAL_UNPRESSED, &accel_pedal_gpio);

  // brake pedal data collection & set to struct
  pedal_calib_sample(&pedal_storage, &(pedal_persist_data.brake_pedal_data), PEDAL_PRESSED, &brake_pedal_gpio);
  pedal_calib_sample(&pedal_storage, &(pedal_persist_data.brake_pedal_data), PEDAL_UNPRESSED, &brake_pedal_gpio);

  // set up persist
  persist_init(&persist_storage, LAST_PAGE, &pedal_persist_data, sizeof(pedal_persist_data), true);

  // commit
  persist_commit(&persist_storage);

  while (true) {
  }
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main() {
#endif
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(pedal_calib, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
