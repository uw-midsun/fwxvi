#pragma once

/************************************************************************************************
 * uart.h
 *
 * UART Library Header file
 *
 * Created: 2024-11-02
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "gpio.h"
#include "uart_mcu.h"

#define UART_MAX_BUFFER_LEN 256U
#define UART_TIMEOUT_MS 10U

typedef enum {
  UART_FLOW_CONTROL_NONE,
  UART_FLOW_CONTROL_RTS,
  UART_FLOW_CONTROL_CTS,
  UART_FLOW_CONTROL_RTS_CTS
} UartFlowControl;

typedef struct {
  GpioAddress tx;
  GpioAddress rx;
  uint32_t baudrate;
  UartFlowControl flow_control;
} UartSettings;

/**
 * @brief   Initialize UART instance, assuming standard 8 bits 1 stop bit
 * @param   uart Specifies which UART port to initialize
 * @param   settings Pointer to UART initialization settings
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 *          STATUS_CODE_RESOURCE_EXHAUSTED if already initialized
 */
StatusCode uart_init(UartPort uart, UartSettings *settings);

/**
 * @brief   Receive data from the UART port
 * @details Receives length bytes of data from UART RX queue.
 *          Receives up to UART_MAX_BUFFER_LEN.
 * @param   uart Specifies which UART port to read from
 * @param   data Pointer to the data buffer
 * @param   len Length of data to receive
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL receiving fails
 *          STATUS_CODE_TIMEOUT if receiving takes too long
 */
StatusCode uart_rx(UartPort uart, uint8_t *data, size_t len);

/**
 * @brief   Transmit data from the UART port
 * @details Sends length bytes of data from UART TX queue to specified UART port.
 *          Sends up to UART_MAX_BUFFER_LEN.
 * @param   uart Specifies which UART port to read from
 * @param   data Pointer to the data buffer
 * @param   len Length of data to receive
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL transmission fails
 *          STATUS_CODE_TIMEOUT if transmission takes too long
 */
StatusCode uart_tx(UartPort uart, uint8_t *data, size_t len);
