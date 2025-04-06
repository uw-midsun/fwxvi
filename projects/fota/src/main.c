/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for fota
 *
 * @date   2025-03-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "fota.h"

int main() {
  mcu_init();

  static UartSettings uart2_settings = {
    .tx = {.port = GPIO_PORT_A, .pin=2},
    .rx = {.port = GPIO_PORT_A, .pin=3},
    .baudrate=115200,
    .flow_control=UART_FLOW_CONTROL_NONE
  };

  uart_init(UART_PORT_2, &uart2_settings);

  while (true) {
  }

  return 0;
}
