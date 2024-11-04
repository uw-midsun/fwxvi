/************************************************************************************************
 * main.c
 *
 * Main file for [PROJECT NAME]
 *
 * Created: [YYYY/MM/DD]
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"

/* Intra-component Headers */

void pre_loop_init() {}

void run_fast_cycle() {}

void run_medium_cycle() {}

void run_slow_cycle() {}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
