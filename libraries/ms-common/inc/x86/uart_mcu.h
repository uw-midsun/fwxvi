#pragma once

/************************************************************************************************
 * uart_mcu.h
 *
 * Header file for MCU specific UART library
 *
 * Created: 2024-11-02
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */

/* Intra-component Headers */

#ifndef NUM_UART_PORTS
#define NUM_UART_PORTS 2U
#endif
typedef enum {
  UART_PORT_1 = 0,
  UART_PORT_2,
} UartPort;
