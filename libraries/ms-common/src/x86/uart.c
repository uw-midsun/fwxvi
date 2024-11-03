/************************************************************************************************
 * uart.c
 *
 * UART Library Source Code
 *
 * Created: 2024-11-02
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <stddef.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "status.h"
#include "uart.h"
#include "uart_mcu.h"

StatusCode uart_init(UartPort uart, UartSettings *settings) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode uart_rx(UartPort uart, uint8_t *data, size_t len) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode uart_tx(UartPort uart, uint8_t *data, size_t len) {
  return STATUS_CODE_UNIMPLEMENTED;
}