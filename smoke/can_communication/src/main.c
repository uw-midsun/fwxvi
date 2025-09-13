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
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "system_can.h"
#include "tasks.h"

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
  LOG_DEBUG("Starting can_communication\n");

  if (can_init(&s_can_storage, &can_settings) != STATUS_CODE_OK) {
    LOG_DEBUG("\nFailed to initialze CAN\n");
    while (true) {
    }
  }

  CanMessage rx_msg;

  CanMessage tx_msg = { .id.raw = 0x123, .data_u8 = { 0xDE, 0xAD, 0xBE, 0xEF }, .dlc = 4U, .extended = false };

  while (true) {
    delay_ms(250U);

    if (can_transmit(&tx_msg) == STATUS_CODE_OK) {
#ifdef MS_PLATFORM_X86
      LOG_DEBUG("Transmitted CAN message -- ID: %u DLC: %u\n", rx_msg.id.raw, rx_msg.dlc);
#else
      LOG_DEBUG("Transmitted CAN message -- ID: %lu DLC: %u\n", tx_msg.id.raw, tx_msg.dlc);
#endif

      for (uint8_t i = 0; i < 8U; i++) {
        delay_ms(10U);
        LOG_DEBUG("Byte %d: 0x%02X\n", i, tx_msg.data_u8[i]);
      }
    }

    delay_ms(250U);

    if (can_receive(&rx_msg) == STATUS_CODE_OK) {
#ifdef MS_PLATFORM_X86
      LOG_DEBUG("Received CAN message -- ID: %u DLC: %u\n", rx_msg.id.raw, rx_msg.dlc);
#else
      LOG_DEBUG("Received CAN message -- ID: %lu DLC: %u\n", rx_msg.id.raw, rx_msg.dlc);
#endif

      for (uint8_t i = 0; i < 8U; i++) {
        delay_ms(10U);
        LOG_DEBUG("Byte %d: 0x%02X\n", i, rx_msg.data_u8[i]);
      }
    }
  }
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main() {
#endif
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(can_communication, TASK_PRIORITY(3U), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
