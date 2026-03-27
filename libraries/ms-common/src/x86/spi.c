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
  Mutex mutex;
} SpiBuffer;

typedef struct {
  SpiSettings settings;
  SpiBuffer spi_tx_buf;
  SpiBuffer spi_rx_buf;
  SpiMode spi_mode;
  volatile uint8_t num_rx_bytes;
} SPIPortData;

// only supported ports for SPI im MPXE are port 1 and port 2
static SPIPortData s_port[NUM_SPI_PORTS] = { [SPI_PORT_1] = {}, [SPI_PORT_2] = {} };

// Initalize Queue for Spi Buffer
static StatusCode s_init_buf(SpiBuffer *buf) {
  buf->queue.num_items = SPI_MAX_NUM_DATA;
  buf->queue.item_size = sizeof(uint8_t);
  buf->queue.storage_buf = buf->buf;
  return queue_init(&buf->queue);
}

StatusCode spi_init(SpiPort spi, const SpiSettings *settings) {
  if (spi >= NUM_SPI_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (settings->baudrate >= NUM_SPI_BAUDRATE) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (settings->mode >= NUM_SPI_MODES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_port[spi].settings = *settings;
  status_ok_or_return(s_init_buf(&s_port[spi].spi_tx_buf));
  status_ok_or_return(s_init_buf(&s_port[spi].spi_rx_buf));

  return STATUS_CODE_OK;
}

StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len) {
  if (spi >= NUM_SPI_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (tx_len > SPI_MAX_NUM_DATA || rx_len > SPI_MAX_NUM_DATA) {
    return STATUS_CODE_INVALID_ARGS;
  }

  status_ok_or_return(spi_write(spi, tx_data, tx_len));
  status_ok_or_return(spi_read(spi, rx_data, rx_len));

  return STATUS_CODE_OK;
}

StatusCode spi_read(SpiPort spi, uint8_t *rx_data, uint8_t rx_len) {
  if (spi >= NUM_SPI_PORTS) return STATUS_CODE_INVALID_ARGS;

  s_port[spi].num_rx_bytes = rx_len;

  for (size_t i = 0; i < rx_len; i++) {
    if (queue_receive(&s_port[spi].spi_rx_buf.queue, &rx_data[i], 0)) {
      queue_reset(&s_port[spi].spi_rx_buf.queue);
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }
  return STATUS_CODE_OK;
}

StatusCode spi_write(SpiPort spi, uint8_t *tx_data, uint8_t tx_len) {
  if (spi >= NUM_SPI_PORTS) return STATUS_CODE_INVALID_ARGS;

  for (size_t i = 0; i < tx_len; i++) {
    if (queue_send(&s_port[spi].spi_tx_buf.queue, &tx_data[i], 0)) {
      queue_reset(&s_port[spi].spi_tx_buf.queue);
      return STATUS_CODE_RESOURCE_EXHAUSTED;
    }
  }

  return STATUS_CODE_OK;
}

StatusCode spi_get_tx_data(SpiPort spi, uint8_t *data, uint8_t len) {
  if (spi >= NUM_SPI_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (len > SPI_MAX_NUM_DATA) {
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

  if (len > SPI_MAX_NUM_DATA) {
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

size_t spi_get_tx_num_bytes(SpiPort spi) {
  if (spi >= NUM_SPI_PORTS) {
    return 0U;
  }

  return s_port[spi].spi_tx_buf.queue.num_items - queue_get_spaces_available(&s_port[spi].spi_tx_buf.queue);
}