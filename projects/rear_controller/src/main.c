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
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "killswitch.h"
#include "precharge.h"
#include "rear_controller.h"

void pre_loop_init() {
  killswitch_init(REAR_CONTROLLER_KILLSWITCH_EVENT, get_1000hz_task());
  precharge_init(REAR_CONTROLLER_PRECHARGE_EVENT, get_10hz_task());
}

void run_1000hz_cycle() {}

void run_10hz_cycle() {}

void run_1hz_cycle() {}

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

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
