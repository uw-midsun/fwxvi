/************************************************************************************************
 * @file   spi.c
 *
 * @brief  SPI Library Source Code
 *
 * @date   2024-12-23
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>
/* Inter-component Headers */

/* Intra-component Headers */
#include "log.h"
#include "ms_semaphore.h"
#include "queues.h"
#include "spi.h"
#include "status.h"

typedef struct {
  uint8_t buf[SPI_MAX_NUM_DATA];
  Queue queue;
} SpiBuffer;

typedef struct {
  SpiSettings settings;
  SpiBuffer spi_tx_buf;
  SpiBuffer spi_rx_buf;
  SpiMode spi_mode;
  // SpiPort current_port;
  volatile uint8_t num_rx_bytes;
  // volatile uint8_t num_tx_bytes;
} SPIPortData;

/* current implementation of spi server/client manager only accepts SPI_PORT_1 and SPI_PORT_2 in MPXE */
// static const int NUM_PORTS = NUM_SPI_PORTS - 1;
static SPIPortData s_port[NUM_SPI_PORTS] = { [SPI_PORT_1] = {}, [SPI_PORT_2] = {} };

// Initalize Queue for Spi Buffer
static StatusCode s_init_buf(SpiBuffer *buf) {
  buf->queue.num_items = SPI_MAX_NUM_DATA;
  buf->queue.item_size = sizeof(uint8_t);
  buf->queue.storage_buf = buf->buf;
  return queue_init(&buf->queue);
}

StatusCode spi_init(SpiPort spi, const SpiSettings *settings) {
  if (spi >= NUM_SPI_PORTS || settings->baudrate >= NUM_SPI_BAUDRATE || settings->mode >= NUM_SPI_MODES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_port[spi].settings = *settings;
  status_ok_or_return(s_init_buf(&s_port[spi].spi_tx_buf));
  status_ok_or_return(s_init_buf(&s_port[spi].spi_rx_buf));

  return STATUS_CODE_OK;
}

StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode spi_get_tx_data(SpiPort spi, uint8_t *data, uint8_t len) {
  if (spi >= NUM_SPI_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  for (uint8_t i = 0; i < len; i++) {
    if (queue_receive(&s_port[spi].spi_tx_buf.queue, &data[i], 0)) {
      queue_reset(&s_port[spi].spi_tx_buf.queue);
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }

  return STATUS_CODE_OK;
}

StatusCode spi_set_rx(SpiPort spi, const uint8_t *data, uint8_t len) {
  if (spi >= NUM_SPI_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  for (uint8_t i = 0; i < len; i++) {
    if (queue_send(&s_port[spi].spi_rx_buf.queue, &data[i], 0)) {
      queue_reset(&s_port[spi].spi_rx_buf.queue);
      return STATUS_CODE_RESOURCE_EXHAUSTED;
    }
  }

  return STATUS_CODE_OK;
}
