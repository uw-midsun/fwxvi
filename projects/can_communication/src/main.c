/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for can_communication
 *
 * @date   2024-11-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "can.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "system_can.h"
#include "tasks.h"
#include "delay.h"

/* Intra-component Headers */
#include "can_communication.h"

static CanStorage s_can_storage = { 0 };

const CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_CAN_COMMUNICATION,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_B, 9 },
  .rx = { GPIO_PORT_B, 8 },
  .loopback = false,
  .silent = false,
};

TASK(can_communication, TASK_STACK_1024) {
  if (can_init(&s_can_storage, &can_settings) != STATUS_CODE_OK) {
    LOG_DEBUG("Failed to initialze CAN\n");
    while (true) {};
  }
  uint8_t can_data[4U] = {0xDE, 0xAD, 0xBE, 0xEF};
  LOG_DEBUG("Starting can_communication\n");
  while (true) {
    LOG_DEBUG("Running\n");
    delay_ms(10);
    can_hw_transmit(123, false, can_data, 4U);
    delay_ms(500);
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
