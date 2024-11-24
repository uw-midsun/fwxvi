/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for CAN communication
 *
 * @date   2024-11-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */

TASK(can_communication, TASK_STACK_512) {
  while (true) {
    LOG_DEBUG("BLINKY\n");
  }
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(can_communication, TASK_PRIORITY(3U), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
