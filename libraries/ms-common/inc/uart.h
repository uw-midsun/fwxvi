#pragma once

/************************************************************************************************
 * @file   uart.h
 *
 * @brief  UART Library Header file
 *
 * @date   2024-11-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "gpio.h"

/**
 * @defgroup UART
 * @brief    UART library
 * @{
 */

/** @brief   UART Port selection */
typedef enum { UART_PORT_1 = 0, UART_PORT_2, NUM_UART_PORTS } UartPort;

/** @brief Maximum size permitted for a UART transaction */
#define UART_MAX_BUFFER_LEN 256U
/** @brief Maximum time permitted for a UART transaction */
#define UART_TIMEOUT_MS 1U

/**
 * @brief   UART Flow control selection
 * @details Flow control is a method of synchronizing UART communication between
 *          2 devices. For most applications, this can be set to UART_FLOW_CONTROL_NONE.
 */
typedef enum {
  UART_FLOW_CONTROL_NONE,   /**< Disable flow control */
  UART_FLOW_CONTROL_RTS,    /**< Enable request-to-send flow control */
  UART_FLOW_CONTROL_CTS,    /**< Enable clear-to-send flow control */
  UART_FLOW_CONTROL_RTS_CTS /**< Enable both request-to-send and clear-to-send flow control */
} UartFlowControl;

/**
 * @brief   UART Settings
 * @details TX Pin will transmit data. RX pin will receive data.
 *          The baudrate is the number of symbols sent per second.
 *          Both devices must agree on the baudrate with a tolerance of +-3%.
 *          Flow control is of type UartFlowControl.
 */
typedef struct {
  GpioAddress tx;               /**< GPIO Pin for UART TX */
  GpioAddress rx;               /**< GPIO Pin for UART RX */
  uint32_t baudrate;            /**< Symbols per second */
  UartFlowControl flow_control; /**< UART Flow control settings */
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
 * @return  STATUS_CODE_OK if receiving succeeded
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
 * @return  STATUS_CODE_OK if transmission succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL transmission fails
 *          STATUS_CODE_TIMEOUT if transmission takes too long
 */
StatusCode uart_tx(UartPort uart, uint8_t *data, size_t len);

/** @} */
