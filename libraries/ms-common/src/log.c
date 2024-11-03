/************************************************************************************************
 * log.c
 *
 * Source code for the logging library used to debug all modules
 *
 * Created: 2024-11-02
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */

/* Intra-component Headers */

#include "log.h"

// Allocating memory for extern variables in .h files
char g_log_buffer[MAX_LOG_SIZE];
Mutex s_log_mutex;

UartSettings log_uart_settings = {
  .tx = { .port = GPIO_PORT_B, .pin = TX_PIN, .alternate_func = GPIO_ALT7_USART1 },
  .rx = { .port = GPIO_PORT_B, .pin = RX_PIN, .alternate_func = GPIO_ALT7_USART1 },
  .baudrate = 115200
};
