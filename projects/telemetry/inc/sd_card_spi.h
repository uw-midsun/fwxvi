#pragma once

/************************************************************************************************
 * @file   sd_card_spi.h
 *
 * @brief  SD Card SPI Library Header File
 *
 * @date   2025-07-05
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Inter-component Headers */
#include "gpio.h"
#include "status.h"

/**
 * @defgroup telemetry
 * @brief    telemetry Firmware
 * @{
 */

/**
 * @brief   SD Card SPI baudrate options
 * @details These prescaler values map to SPI peripheral baudrate divisors.
 *          The actual baudrate is calculated as APB_Frequency / Prescaler.
 *          On STM32, prescalers must be powers of 2.
 */
typedef enum {
  SD_SPI_BAUDRATE_312_5KHZ = 0, /**< 312.5 kHz */
  SD_SPI_BAUDRATE_625KHZ,       /**< 625 kHz */
  SD_SPI_BAUDRATE_1_25MHZ,      /**< 1.25 MHz */
  SD_SPI_BAUDRATE_2_5MHZ,       /**< 2.5 MHz */
  SD_SPI_BAUDRATE_5MHZ,         /**< 5 MHz */
  SD_SPI_BAUDRATE_10MHZ,        /**< 10 MHz */
  SD_SPI_BAUDRATE_20MHZ,        /**< 20 MHz */
  SD_SPI_BAUDRATE_40MHZ,        /**< 40 MHz */
  NUM_SD_SPI_BAUDRATES          /**< Number of baudrate options */
} SdSpiBaudrate;

/**
 * @brief   SPI modes (CPOL and CPHA settings)
 */
typedef enum {
  SD_SPI_MODE_0 = 0, /**< CPOL: 0, CPHA: 0 — idle low, sample rising edge */
  SD_SPI_MODE_1,     /**< CPOL: 0, CPHA: 1 — idle low, sample falling edge */
  SD_SPI_MODE_2,     /**< CPOL: 1, CPHA: 0 — idle high, sample falling edge */
  SD_SPI_MODE_3,     /**< CPOL: 1, CPHA: 1 — idle high, sample rising edge */
  NUM_SD_SPI_MODES   /**< Number of SPI modes */
} SdSpiMode;

/**
 * @brief   SD SPI port options
 */
typedef enum {
  SD_SPI_PORT_1 = 0,
  SD_SPI_PORT_2,
  SD_SPI_PORT_3,
  NUM_SD_SPI_PORTS
} SdSpiPort;

/**
 * @brief   Configuration settings for initializing an SD SPI port
 */
typedef struct {
  SdSpiBaudrate baudrate; /**< Desired SPI baudrate */
  SdSpiMode mode;         /**< Clock polarity and phase mode */
  GpioAddress mosi;       /**< Master Out, Slave In (STM32 → SD) */
  GpioAddress miso;       /**< Master In, Slave Out (SD → STM32) */
  GpioAddress sclk;       /**< SPI clock pin */
  GpioAddress cs;         /**< Chip Select pin (active low) */
} SdSpiSettings;

/**
 * @brief   Initialize SD SPI port with given settings
 * @details Note that STM32 SPI baudrate prescalers are powers of 2, so the resulting
 *          baudrate may not match the requested value exactly. Check resulting timing if critical
 * @param   spi SPI port to initialize
 * @param   settings Pointer to configuration settings
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_INVALID_ARGS if parameters are invalid
 *          STATUS_CODE_RESOURCE_EXHAUSTED if already initialized
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 */
StatusCode sd_spi_init(SdSpiPort spi, const SdSpiSettings *settings);

/**
 * @brief   Transmit data to SD card via SPI
 * @details Sends |tx_len| bytes from |tx_data| over SPI without changing the CS line.
 *          Received bytes are discarded
 * @param   spi SPI port to use
 * @param   tx_data Pointer to data to transmit
 * @param   tx_len Number of bytes to transmit
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_INVALID_ARGS if inputs are invalid
 *          STATUS_CODE_UNINITIALIZED if SPI is not initialized
 *          STATUS_CODE_INTERNAL_ERROR if HAL transmit fails
 */
StatusCode sd_spi_tx(SdSpiPort spi, uint8_t *tx_data, size_t tx_len);

/**
 * @brief   Receive data from SD card via SPI
 * @details Reads |rx_len| bytes into |rx_data|. Sends the specified |placeholder| byte
 *          while receiving (commonly 0xFF)
 * @param   spi SPI port to use
 * @param   rx_data Pointer to buffer to receive data
 * @param   rx_len Number of bytes to receive
 * @param   placeholder Byte to transmit during receive
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_INVALID_ARGS if inputs are invalid
 *          STATUS_CODE_UNINITIALIZED if SPI is not initialized
 *          STATUS_CODE_INTERNAL_ERROR if HAL receive fails
 */
StatusCode sd_spi_rx(SdSpiPort spi, uint8_t *rx_data, size_t rx_len, uint8_t placeholder);

/**
 * @brief   Exchange data over SPI
 * @details Simultaneously transmit and receive data. Lengths may differ. Transmits 0xFF if no TX data
 * @param   spi SPI port to use
 * @param   tx_data Pointer to data to transmit (can be NULL)
 * @param   tx_len Number of bytes to transmit
 * @param   rx_data Pointer to buffer for received data (can be NULL)
 * @param   rx_len Number of bytes to receive
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_INVALID_ARGS if parameters are invalid
 *          STATUS_CODE_UNINITIALIZED if SPI not initialized
 *          STATUS_CODE_INTERNAL_ERROR if SPI transfer fails
 */
StatusCode sd_spi_exchange(SdSpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len);

/**
 * @brief   Set the CS (chip select) GPIO state
 * @param   spi SPI port
 * @param   state Desired CS state (GPIO_STATE_LOW to select)
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_INVALID_ARGS if port is invalid
 *          STATUS_CODE_UNINITIALIZED if SPI not initialized
 */
StatusCode sd_spi_cs_set_state(SdSpiPort spi, GpioState state);

/**
 * @brief   Get the current state of the CS (chip select) pin
 * @param   spi SPI port
 * @return  GPIO_STATE_HIGH if pin is high
 *          GPIO_STATE_LOW if pin is low
 */
GpioState sd_spi_cs_get_state(SdSpiPort spi);

/**
 * @brief   Change the baudrate of an initialized SPI port
 * @details Reconfigures the SPI peripheral to operate at a different baudrate
 * @param   spi SPI port
 * @param   baudrate Desired new baudrate
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_INVALID_ARGS if arguments are invalid
 *          STATUS_CODE_UNINITIALIZED if SPI not initialized
 */
StatusCode sd_spi_set_frequency(SdSpiPort spi, SdSpiBaudrate baudrate);

/** @} */
