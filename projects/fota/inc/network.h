#pragma once

/************************************************************************************************
 * @file   network.h
 *
 * @brief  Header file for network layer Tx Rx UART implementation for firmware over the air (FOTA) updates
 *
 * @date   2025-03-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_error.h"
#include "network_buffer.h"

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

/** @brief   UART Port selection */
typedef enum { UART_PORT_1 = 0, UART_PORT_2, NUM_UART_PORTS } UartPort;

/** @brief Maximum size permitted for a UART transaction */
#define FOTA_UART_MAX_BUFFER_LEN 256U
/** @brief Maximum time permitted for a FOTA UART transaction */
#define FOTA_UART_TIMEOUT_MS 10U

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
typedef enum {
  GPIO_PORT_A = 0, /**< Gpio port A */
  GPIO_PORT_B,     /**< Gpio port B */
  NUM_GPIO_PORTS   /**< Number of Gpio Ports */
} GpioPort;

typedef struct {
  GpioPort port;
  uint32_t pin;
} GpioAddress;

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
 * @param   network_buffer Pointer to the network buffer for storing recieved data
 * @return  FOTA_ERROR_SUCCESS if initialization succeeded
 *          FOTA_ERROR_INVALID_ARGS if one of the parameters are incorrect
 *          FOTA_ERROR_INTERNAL_ERROR if HAL initialization fails
 *          FOTA_RESOURCE_EXHAUSTED if already initialized
 */
FotaError network_init(UartPort uart, UartSettings *settings, NetworkBuffer *network_buffer);

/**
 * @brief   Read data from the network buffer
 * @details Receives len bytes of data from the network buffer.
 *          Receives up to NETWORK_BUFFER_SIZE.
 * @param   uart Specifies which UART port the data is coming from
 * @param   data Pointer to the read data buffer
 * @param   len Length of data to receive
 * @return  FOTA_ERROR_SUCCESS if receiving succeeded
 *          FOTA_ERROR_INVALID_ARGS if one of the parameters are incorrect
 *          FOTA_RESOURCE_EXHAUSTED if uart port is not initialized
 *          FOTA_ERROR_INTERNAL_ERROR if reading from network buffer fails or takes too long
 */
FotaError network_read(UartPort uart, uint8_t *data, size_t len);

/**
 * @brief   Transmit data from the UART port
 * @details Sends length bytes of data from UART TX queue to specified UART port.
 *          Sends up to UART_MAX_BUFFER_LEN.
 *          Updates start time of tx command and sets transmitting status to true, check timeout status in main loop.
 * @param   uart Specifies which UART port to send from
 * @param   data Pointer to the tx data buffer
 * @param   len Length of data to send
 * @return  FOTA_ERROR_SUCCESS if transmission succeeded
 *          FOTA_ERROR_INVALID_ARGS if one of the parameters are incorrect
 *          FOTA_RESOURCE_EXHAUSTED if uart port is not initialized
 *          FOTA_ERROR_INTERNAL_ERROR if HAL transmission fails
 */
FotaError network_tx(UartPort uart, uint8_t *data, size_t len);

/**
 * @brief   Checks if tx or rx command has timed out
 * @details Subtracts the difference of current clock time and clock time when command started
 * @param   is_tx if it is checking timeout for a tx or an rx command
 * @return  true if time elapsed is greater than FOTA_UART_TIMEOUT_MS
 *          false if time elapsed is less than FOTA_UART_TIMEOUT_MS
 */
bool is_network_timeout(bool is_tx);

#ifdef MS_PLATFORM_X86

/**
 * @brief   Sets the RX data in the network buffer to be read
 * @param   data Pointer to the simulated data buffer
 * @param   len Length of data to send
 * @return  FOTA_ERROR_SUCCESS if data is simulated succeeded
 *          FOTA_ERROR_INVALID_ARGS if one of the parameters are incorrect
 */
FotaError network_sim_set_rx_data(uint8_t *data, size_t len);

/**
 * @brief   Gets the TX data that was just transmitted
 * @param   data Pointer to the simulated buffer storing TX data
 * @return  FOTA_ERROR_SUCCESS if simulated data is read succeeded
 *          FOTA_ERROR_INVALID_ARGS if one of the parameters are incorrect
 */
FotaError network_sim_get_tx_data(uint8_t *data);

#endif

/** @} */
