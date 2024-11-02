#pragma once

/************************************************************************************************
 * uart_mcu.h
 *
 * Header file for MCU specific UART library
 *
 * Created: 2024-10-27
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

typedef enum {
  UART_PORT_1 = 0,
  UART_PORT_2,
  UART_PORT_3,
  NUM_UART_PORTS,
} UartPort;
