/************************************************************************************************
 * main.c
 *
 * Main file for CAN communication
 *
 * Created: 2024-11-03
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */

TASK(CANCommunication, TASK_STACK_512) {
  while (true) {
    LOG_DEBUG("BLINKY\n");
  }
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(CANCommunication, TASK_PRIORITY(3U), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
