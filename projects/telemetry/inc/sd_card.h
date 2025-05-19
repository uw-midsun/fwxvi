/* Standard library Headers */
#include <stddef.h>
#include <stdint.h> 
#include "stm32l433xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_spi.h" /* !DELETE: Use this gang */
#include "stm32l4xx_hal_gpio.h"
#include <status.h>

// The block size on the SD card
#define SD_BLOCK_SIZE (512)

/** @brief  SPI Port selection */
typedef enum {
  SPI_PORT_1 = 0, /**< CS: PA4, SCLK: PA5, SDI: PA6, and SDO: PA7 */
  SPI_PORT_2,     /**< CS: PB12, SCLK: PB13, SDI: PB14, and SDO: PB15 */
  SPI_PORT_3,     /**< CS: PA15, SCLK: PB3, SDI: PB4, and SDO: PB5 */
  NUM_SPI_PORTS,  /**< Number of SPI Ports */
} SpiPort;


// For SDHC and SDXC cards, the address provided to these functions should be the block address

// Initialize the SD card on a given SPI port
StatusCode sd_card_init(SpiPort spi);

// Read block from the SD card. |dest| is where the read blocks will be written into. Make sure that
// this buffer is large enough for the content
StatusCode sd_read_blocks(SpiPort spi, uint8_t *dest, uint32_t readAddr, uint32_t numberOfBlocks);

// Write blocks to the SD card from |src| to a location on the SD card specified by |writeAddr|
StatusCode sd_write_blocks(SpiPort spi, uint8_t *src, uint32_t writeAddr, uint32_t numberOfBlocks);

// Determines whether the SD card is ready in on a given SPI port
StatusCode sd_is_initialized(SpiPort spi);