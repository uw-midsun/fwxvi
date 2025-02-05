/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for crc_api
 *
 * @date   2025-02-05
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "status.h"
#include "delay.h"
#include "crc.h"

/* Intra-component Headers */

TASK(crc_api, TASK_STACK_1024) {
  while (true) {}
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(crc_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}