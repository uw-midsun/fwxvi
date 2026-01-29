/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for rear_controller_power_path_manager
 *
 * @date   2026-01-21
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */
#include "power_path_manager.h"

#define CYCLE_PERIOD_MS 2000U
#define LOG_DB_DELAY 10U

static RearControllerStorage rear_storage;
static RearControllerConfig rear_config = { 0 };

TASK(power_path_manager_run_cycle, TASK_STACK_1024) {
  LOG_DEBUG("Initializing power path manager...\r\n");
  delay_ms(1000U);
  rear_storage.config = &rear_config;
  StatusCode status = power_path_manager_init(&rear_storage);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("Power path manager initialized\r\n");
  } else {
    LOG_DEBUG("Power path manager cannot be initialized\r\n");
  }
  delay_ms(500U);

  while (true) {
    StatusCode status = power_path_manager_run();
    if (status == STATUS_CODE_OK) {
      LOG_DEBUG("Power path manager running\r\n");
    } else {
      LOG_DEBUG("Power path manager failing to run\r\n");
    }

    delay_ms(LOG_DB_DELAY);
    LOG_DEBUG("PCS Valid: %d\r\n", rear_storage.pcs_valid);
    LOG_DEBUG("AUX Valid: %d\r\n", rear_storage.aux_valid);

    delay_ms(LOG_DB_DELAY);
    LOG_DEBUG("PCS Voltage: %ld mV\n", rear_storage.pcs_voltage);
    LOG_DEBUG("AUX Voltage: %ld mV\n", rear_storage.aux_voltage);
    LOG_DEBUG("PCS Current: %ld mA\n", rear_storage.pcs_current);
    LOG_DEBUG("AUX Current: %ld mA\n", rear_storage.aux_current);
    delay_ms(CYCLE_PERIOD_MS);
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

  tasks_init_task(power_path_manager_run_cycle, TASK_PRIORITY(3), NULL);
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
