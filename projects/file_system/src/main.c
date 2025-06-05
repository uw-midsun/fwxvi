/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for file_system
 *
 * @date   2025-06-05
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "master_tasks.h"

/* Intra-component Headers */
#include "file_system.h"

void pre_loop_init() {}

void run_1000hz_cycle() {}

void run_10hz_cycle() {}

void run_1hz_cycle() {}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}