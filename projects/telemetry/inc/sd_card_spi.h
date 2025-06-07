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

// For SDHC and SDXC cards, the address provided to these functions should be the block address

// Initialize the SD card on a given SPI port
StatusCode sd_card_init(SdSpiPort spi, SdSpiSettings *settings);

// Read block from the SD card. |dest| is where the read blocks will be written into. Make sure that
// this buffer is large enough for the content
StatusCode sd_read_blocks(SdSpiPort spi, uint8_t *dest, uint32_t readAddr, uint32_t numberOfBlocks);

// Write blocks to the SD card from |src| to a location on the SD card specified by |writeAddr|
StatusCode sd_write_blocks(SdSpiPort spi, uint8_t *src, uint32_t writeAddr, uint32_t numberOfBlocks);

// Determines whether the SD card is ready in on a given SPI port
StatusCode sd_is_initialized(SdSpiPort spi);

/** @} */
