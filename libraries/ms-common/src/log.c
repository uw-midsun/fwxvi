/************************************************************************************************
 * @file   log.c
 *
 * @brief  Source code for the logging library used to debug all modules
 *
 * @date   2024-11-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

#include "log.h"

// Allocating memory for extern variables in .h files
char g_log_buffer[MAX_LOG_SIZE];
Mutex s_log_mutex;

UartSettings log_uart_settings = { .tx = { .port = GPIO_PORT_B, .pin = TX_PIN }, .rx = { .port = GPIO_PORT_B, .pin = RX_PIN }, .baudrate = 115200 };
