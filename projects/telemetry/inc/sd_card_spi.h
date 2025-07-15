#pragma once

/************************************************************************************************
 * @file    sd_card_spi.h
 *
 * @brief   Sd Card Spi
 *
 * @date    2025-06-07
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <status.h>
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */
#include "gpio.h"

/* Intra-component Headers */

/**
 * @defgroup telemetry
 * @brief    telemetry Firmware
 * @{
 */

/** @brief  SPI Port selection */
typedef enum {
  SPI_PORT_1 = 0, /**< CS: PA4, SCLK: PA5, SDI: PA6, and SDO: PA7 */
  SPI_PORT_2,     /**< CS: PB12, SCLK: PB13, SDI: PB14, and SDO: PB15 */
  SPI_PORT_3,     /**< CS: PA15, SCLK: PB3, SDI: PB4, and SDO: PB5 */
  NUM_SPI_PORTS,  /**< Number of SPI Ports */
} SdSpiPort;

typedef enum {
  SD_SPI_MODE_0 = 0,  // CPOL: 0 CPHA: 0
  SD_SPI_MODE_1,      // CPOL: 0 CPHA: 1
  SD_SPI_MODE_2,      // CPOL: 1 CPHA: 0
  SD_SPI_MODE_3,      // CPOL: 1 CPHA: 1
  NUM_SD_SPI_MODES,
} SdSpiMode;

typedef struct {
  uint32_t baudrate;
  SdSpiMode mode;

  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;
  GpioAddress cs;
} SdSpiSettings;

/*  Constant + Macros */
#define SD_BLOCK_SIZE 512U
#define SD_SPI_INIT_LOW_FREQ_HZ 400000U
#define SD_SPI_INIT_HIGH_FREQ_HZ 25000000U

/**
 * @brief   Initialize the SD card over SPI
 * @param   p SPI port to use
 * @param   settings Pointer to SPI settings
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INTERNAL_ERROR on failure
 */
StatusCode sd_card_spi_init(SdSpiPort p, SdSpiSettings *settings);

/**
 * @brief   Read one or more 512-byte blocks from the SD card
 * @param   p SPI port to use
 * @param   dst Destination buffer to store the data
 * @param   lba Starting block address (logical block address)
 * @param   number_of_blocks Number of blocks to read
 * @return  STATUS_CODE_OK if read succeeded
 *          STATUS_CODE_INTERNAL_ERROR on command failure
 *          STATUS_CODE_TIMEOUT if read timed out
 */
StatusCode sd_read_blocks(SdSpiPort p, uint8_t *dst, uint32_t lba, uint32_t number_of_blocks);

/**
 * @brief Write blocks to SD card from src to lba
 * @param p SPI port to use
 * @param src pointer to source data buffer in RAM
 * @param lba Starting block address (logical block address)
 * @param number_of_blocks Number of blocks to write
 * @return STATUS_CODE_OK if write succeeded
 *         STATUS_CODE_INTERNAL_ERROR on command failure
 *         STATUS_CODE_TIMEOUT if write timed out
 */
StatusCode sd_write_blocks(SdSpiPort p, uint8_t *src, uint32_t lba, uint32_t number_of_blocks);

/**
 * @brief If SD card is initialized and ready for use on given SPI port
 * @param p SPI port to use
 * @return STATUS_CODE_OK if it is initialized
 *         STATUS_CODE_INTERNAL_ERROR if error with CMD13
 *         STATUS_CODE_RESOURCE_EXHAUSTED if the card is busy after 10 ms
 */
StatusCode sd_is_initialized(SdSpiPort p);

/** @} */
