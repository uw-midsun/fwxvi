/************************************************************************************************
 * main.c
 *
 * Main file for master tasks smoke
 *
 * Created: 2024-11-13
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */

void pre_loop_init() {
  LOG_DEBUG("Pre-loop initialization\n");
}

void run_100hz_cycle() {
  LOG_DEBUG("Running 100Hz cycle\n");
}

void run_10hz_cycle() {
  LOG_DEBUG("Running 10hz cycle\n");
}

void run_1hz_cycle() {
  LOG_DEBUG("Running 1hz cycle\n");
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
