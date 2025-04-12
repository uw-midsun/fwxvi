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
#include <stdbool.h>
// #include "log.h"

/* Intra-component Headers */
#include "network.h"

int main() {
  HAL_Init();

  static UartSettings uart2_settings = {
    .tx = {.port = GPIO_PORT_A, .pin=GPIO_PIN_2},
    .rx = {.port = GPIO_PORT_A, .pin=GPIO_PIN_3},
    .baudrate=115200,
    .flow_control=UART_FLOW_CONTROL_NONE
  };

  network_init(UART_PORT_2, &uart2_settings);

  uint8_t tx_buffer[] = "Hello, World!";
  network_tx(UART_PORT_2, tx_buffer, sizeof(tx_buffer));

  while (true) {
  }

  return 0;
}
