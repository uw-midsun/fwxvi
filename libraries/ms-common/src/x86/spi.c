/************************************************************************************************
 * @file   spi.c
 *
 * @brief  SPI Library Source Code
 *
 * @date   2024-12-23
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdio.h>
#include <stdbool.h>
/* Inter-component Headers */

/* Intra-component Headers */
#include "spi.h"
#include "ms_semaphore.h"
#include "queues.h"
#include "status.h"

typedef struct {
  uint8_t buf[SPI_MAX_NUM_DATA];
  Queue queue;
  Mutex mutex;
} SpiBuffer;


typedef struct {
  SpiSettings settings;
  SpiBuffer spi_tx_buf;
  SpiBuffer spi_rx_buf;
  SpiMode curr_mode;
  SpiPort current_port;
  volatile uint8_t num_rx_bytes;
} SPIPortData;

/* current implementation of spi server/client manager only accepts SPI_PORT_1 and SPI_PORT_2 */
static const int NUM_PORTS = NUM_SPI_PORTS - 1;
static SPIPortData s_port[NUM_PORTS] = { [SPI_PORT_1] = {}, [SPI_PORT_2] = {} };

spi_init(SpiPort spi, const SpiSettings *settings) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode spi_get_tx_data(SpiPort spi, uint8_t *data, uint8_t len) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode spi_set_rx(SpiPort spi, uint8_t *data, uint8_t len) {
  return STATUS_CODE_UNIMPLEMENTED;
}
