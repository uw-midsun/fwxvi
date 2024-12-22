/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for can_communication
 *
 * @date   2024-11-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "can.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "can_communication.h"

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0U,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
  .silent = false,
};

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
