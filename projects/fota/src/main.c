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
#include "network_buffer.h"
#include "fota_jump_app.h"
#include "fota_timeout.h"

int main() {
  static UartSettings uart2_settings = {
    .tx = { .port = GPIO_PORT_A, .pin = GPIO_PIN_2 }, .rx = { .port = GPIO_PORT_A, .pin = GPIO_PIN_3 }, .baudrate = 115200, .flow_control = UART_FLOW_CONTROL_NONE
  };

  NetworkBuffer network_buffer;
  network_init(UART_PORT_2, &uart2_settings, &network_buffer);

  uint8_t tx_buffer[] = "Hello, World!";
  network_tx(UART_PORT_2, tx_buffer, sizeof(tx_buffer));

  uint8_t rx_data[8];
  network_read(UART_PORT_2, rx_data, sizeof(rx_data));

  while (true) {
    if (isTimeout(true)) {
      // tx timeout, rx timeout is built into function to return error if timeout
    }
    if (fota_is_timed_out()) {
      bootloader_jump_app();
    }
  }

  return 0;
}
