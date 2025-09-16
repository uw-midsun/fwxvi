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
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"

/* Intra-component Headers */
#include "pedal_calib.h"

TASK(pedal_calib, TASK_STACK_1024) {
  while (true) {}
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
