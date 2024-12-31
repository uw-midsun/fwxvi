/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for bms_carrier
 *
 * @date   2024-12-25
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
#include "bms_carrier.h"

void pre_loop_init() {}

void run_1000hz_cycle() {
  run_can_rx_all();

  run_can_tx_fast();
}

void run_10hz_cycle() {
  run_can_tx_medium();
}

void run_1hz_cycle() {
  run_can_tx_slow();
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
