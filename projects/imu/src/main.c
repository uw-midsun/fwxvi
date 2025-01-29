/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for imu
 *
 * @date   2025-01-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "master_tasks.h"
#include "can.h"
#include "system_can.h"

/* Intra-component Headers */
#include "imu.h"

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_IMU,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_B, 8 },
  .rx = { GPIO_PORT_B, 9 },
  .loopback = false,
  .silent = false,
}
;
void pre_loop_init() {}

void run_1000hz_cycle() {
  run_can_rx_all();

  run_can_tx_fast();
}
void run_10hz_cycle() {
  run_can_tx_medium();
  LOG_DEBUG("MEDIUM CYCLE!\n");
}

void run_1hz_cycle() {
  run_can_tx_slow();
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  can_init(&s_can_storage, &can_settings);

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}