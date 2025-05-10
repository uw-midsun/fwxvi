#pragma once

/************************************************************************************************
 * @file   spi.h
 *
 * @brief  SPI Library Header file
 *
 * @date   2024-11-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "gpio.h"
#include "status.h"

/**
 * @defgroup SPI
 * @brief    SPI library
 * @{
 */

/** @brief  Maximum time permitted for a SPI transaction */
#define SPI_TIMEOUT_MS 100U
/** @brief  None of our SPI transactions should need to be longer than 32 bytes */
#define SPI_MAX_NUM_DATA 32U

/** @brief  SPI Port selection */
typedef enum {
  SPI_PORT_1 = 0, /**< CS: PA4, SCLK: PA5, SDI: PA6, and SDO: PA7 */
  SPI_PORT_2,     /**< CS: PB12, SCLK: PB13, SDI: PB14, and SDO: PB15 */
  SPI_PORT_3,     /**< CS: PA15, SCLK: PB3, SDI: PB4, and SDO: PB5 */
  NUM_SPI_PORTS,  /**< Number of SPI Ports */
} SpiPort;

/**
 * @brief   SPI baudrate type
 * @details These values are calculated with the following formula
 *          APB_Frequency / Spi_Prescalers. Where APB frequency is 80MHz set in the mcu.c file
 *          And Spi_prescalers are powers of 2 between 2^1 and 2^8
 */
typedef enum {
  SPI_BAUDRATE_312_5KHZ = 0, /**< 312.5 kHz */
  SPI_BAUDRATE_625KHZ,       /**< 625 kHz */
  SPI_BAUDRATE_1_25MHZ,      /**< 1.25 MHz */
  SPI_BAUDRATE_2_5MHZ,       /**< 2.5 MHz */
  SPI_BAUDRATE_5MHZ,         /**< 5 MHz */
  SPI_BAUDRATE_10MHZ,        /**< 10 MHz */
  SPI_BAUDRATE_20MHZ,        /**< 20 MHz */
  SPI_BAUDRATE_40MHZ,        /**< 40 MHz */
  NUM_SPI_BAUDRATE           /**< Number of Spi Baudrates */
} SpiBaudrate;

/** @brief   SPI Mode selection */
typedef enum {
  SPI_MODE_0 = 0, /**< CPOL: 0 CPHA: 0. Sampled on rising edge. Idle low */
  SPI_MODE_1,     /**< CPOL: 0 CPHA: 1. Sampled on falling edge. Idle low */
  SPI_MODE_2,     /**< CPOL: 1 CPHA: 0. Sampled on falling edge. Idle high */
  SPI_MODE_3,     /**< CPOL: 1 CPHA: 1. Sampled on rising edge. Idle high */
  NUM_SPI_MODES,  /**< Number of SPI Modes */
} SpiMode;

/**
 * @brief   SPI settings struct
 * @details Typical SPI baudrates are around 1MHz. Mode is selected based off SpiMode.
 *          SDO is the data output pin from the perspective of the STM32.
 *          SDI is the data input pin from the perspective of the STM32.
 *          SCLK is a shared clock signal between the controller and target.
 *          CS is the chip select pin which acts as an enable signal.
 */
typedef struct {
  SpiBaudrate baudrate;
  SpiMode mode;
  GpioAddress sdo;
  GpioAddress sdi;
  GpioAddress sclk;
  GpioAddress cs;
} SpiSettings;

/**
 * @brief   Initialize SPI instance
 * @details Prescalers on STM32 must be a power of 2, so the actual baudrate
 *          may not be as exactly as requested.
 * @param   spi Specifies which SPI port to initialize
 * @param   settings Pointer to SPI initialization settings
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 *          STATUS_CODE_RESOURCE_EXHAUSTED if already initialized
 */
StatusCode spi_init(SpiPort spi, const SpiSettings *settings);

/**
 * @brief   Performs a SPI data exchange
 * @details Half-duplex communication where it transmits first, then receives. It will
 *          first pull the CS low, transmit the data, then receive data, and finally
 *          pull CS high.
 * @param   spi Specifies which SPI port performs the exchange
 * @param   tx_data Pointer to a buffer to transmit data
 * @param   tx_len Length of the data to transmit
 * @param   rx_data Pointer to a buffer to receive data
 * @param   rx_len Length of the data to receive
 * @return  STATUS_CODE_OK if data exchange succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL transmission/receiving fails
 *          STATUS_CODE_TIMEOUT if transmission/receiving takes too long
 */
StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len);

/**
 * @brief   Transmit data from the SPI port
 * @details This method will only send data, and will discard the response bytes
 * @param   spi Specifies which SPI port performs the transmit
 * @param   tx_data Pointer to a buffer to transmit data
 * @param   tx_len Length of the data to transmit
 */
#define spi_tx(spi, tx_data, tx_len) spi_exchange(spi, tx_data, tx_len, NULL, 0U)

/**
 * @brief   Receive data from the SPI port
 * @details This method will only receive data, and will not send any data
 * @param   spi Specifies which SPI port performs the receive
 * @param   rx_data Pointer to a buffer to receive data
 * @param   rx_len Length of the data to receive
 */
#define spi_rx(spi, rx_data, rx_len) spi_exchange(spi, NULL, 0U, rx_data, rx_len)

#ifdef MS_PLATFORM_X86

/**
 * @brief   Gets data from the spi TX queue
 * @param   i2c Specifies which SPI port to read from
 * @param   data Pointer to a buffer of data to fill
 * @param   len Length of the data to retrieve
 * @return  STATUS_CODE_OK if data is retrieved successfully
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode spi_get_tx_data(SpiPort spi, uint8_t *data, uint8_t len);

/**
 * @brief   Sets SPI RX queue with the given data
 * @param   spi Specifies which SPI port to update
 * @param   data Pointer to a buffer of data to set
 * @param   len Length of the data to set
 * @return  STATUS_CODE_OK if data is set successfully
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode spi_set_rx(SpiPort spi, uint8_t *data, uint8_t len);

#endif

/** @} */
