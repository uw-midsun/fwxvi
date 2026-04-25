/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for rear_controller
 *
 * @date   2025-05-28
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "can.h"
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "bps_fault.h"
#include "cell_sense.h"
#include "current_sense.h"
#include "killswitch.h"
#include "precharge.h"
#include "rear_controller.h"
#include "rear_controller_config.h"
#include "rear_controller_getters.h"
#include "rear_controller_state_manager.h"

RearControllerStorage rear_controller_storage = { 0U };

RearControllerConfig rear_controller_config = {
  .series_count = REAR_CONTROLLER_SERIES_COUNT,
  .parallel_count = REAR_CONTROLLER_PARALLEL_COUNT,
  .cell_capacity_Ah = REAR_CONTROLLER_CELL_CAPACITY_AH,
};

void pre_loop_init() {}

void run_1000hz_cycle() {
  run_can_rx_all();
  killswitch_run();
  precharge_run();
}

void run_10hz_cycle() {
  rear_controller_update_state_manager_medium_cycle();
  log_cell_sense();
  run_can_tx_medium();
}

void run_1hz_cycle() {
  // bps_fault_commit();
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

  rear_controller_init(&rear_controller_storage, &rear_controller_config);

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
