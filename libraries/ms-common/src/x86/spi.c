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
#include "FreeRTOS.h"
#include "log.h"
#include "ms_semaphore.h"
#include "queues.h"
#include "semphr.h"
#include "spi.h"
#include "status.h"
#include "task.h"

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

#define SPI_HOST_DEVICE_LIMIT 4U
static StaticSemaphore_t s_transaction_mutex[NUM_SPI_PORTS];
static SemaphoreHandle_t s_transaction_handle[NUM_SPI_PORTS];
static TaskHandle_t s_transaction_owner[NUM_SPI_PORTS];
static bool s_initialized[NUM_SPI_PORTS];
static GpioAddress s_devices[NUM_SPI_PORTS][SPI_HOST_DEVICE_LIMIT];
static size_t s_device_count[NUM_SPI_PORTS];
static GpioAddress s_selected[NUM_SPI_PORTS];

// Initalize Queue for Spi Buffer
static StatusCode s_init_buf(SpiBuffer *buf) {
  buf->queue.num_items = SPI_MAX_NUM_DATA;
  buf->queue.item_size = sizeof(uint8_t);
  buf->queue.storage_buf = buf->buf;
  return queue_init(&buf->queue);
}

StatusCode spi_init(SpiPort spi, const SpiSettings *settings) {
  if ((unsigned)spi >= NUM_SPI_PORTS || settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (settings->baudrate >= NUM_SPI_BAUDRATE) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (settings->mode >= NUM_SPI_MODES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_initialized[spi]) return STATUS_CODE_RESOURCE_EXHAUSTED;
  s_port[spi].settings = *settings;
  status_ok_or_return(s_init_buf(&s_port[spi].spi_tx_buf));
  status_ok_or_return(s_init_buf(&s_port[spi].spi_rx_buf));
  status_ok_or_return(gpio_init_pin(&settings->cs, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH));
  s_transaction_handle[spi] = xSemaphoreCreateMutexStatic(&s_transaction_mutex[spi]);
  if (!s_transaction_handle[spi]) return STATUS_CODE_RESOURCE_EXHAUSTED;
  s_devices[spi][0] = settings->cs;
  s_device_count[spi] = 1U;
  s_initialized[spi] = true;

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

static bool s_valid_device(SpiPort spi, const SpiSettings *settings) {
  return (unsigned)spi < NUM_SPI_PORTS && settings && (unsigned)settings->mode < NUM_SPI_MODES && (unsigned)settings->baudrate < NUM_SPI_BAUDRATE;
}
StatusCode spi_register_device(SpiPort spi, const SpiSettings *settings) {
  if (!s_valid_device(spi, settings)) return STATUS_CODE_INVALID_ARGS;
  if (!s_initialized[spi]) return STATUS_CODE_UNINITIALIZED;
  if (xSemaphoreTake(s_transaction_handle[spi], pdMS_TO_TICKS(1000U)) != pdTRUE) return STATUS_CODE_TIMEOUT;
  StatusCode result = STATUS_CODE_OK;
  const SpiSettings *bus = &s_port[spi].settings;
  if (settings->sdo.port != bus->sdo.port || settings->sdo.pin != bus->sdo.pin || settings->sdi.port != bus->sdi.port || settings->sdi.pin != bus->sdi.pin || settings->sclk.port != bus->sclk.port ||
      settings->sclk.pin != bus->sclk.pin)
    result = STATUS_CODE_INVALID_ARGS;
  else {
    size_t i;
    for (i = 0; i < s_device_count[spi]; ++i) {
      if (s_devices[spi][i].port == settings->cs.port && s_devices[spi][i].pin == settings->cs.pin) break;
    }
    if (i == s_device_count[spi]) {
      if (i == SPI_HOST_DEVICE_LIMIT)
        result = STATUS_CODE_RESOURCE_EXHAUSTED;
      else {
        result = gpio_init_pin(&settings->cs, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
        if (result == STATUS_CODE_OK) s_devices[spi][s_device_count[spi]++] = settings->cs;
      }
    }
  }
  xSemaphoreGive(s_transaction_handle[spi]);
  return result;
}
static StatusCode s_host_begin(SpiPort spi, const SpiSettings *settings, bool select) {
  if (!s_valid_device(spi, settings)) return STATUS_CODE_INVALID_ARGS;
  if (!s_initialized[spi]) return STATUS_CODE_UNINITIALIZED;
  size_t i;
  for (i = 0; i < s_device_count[spi]; ++i) {
    if (s_devices[spi][i].port == settings->cs.port && s_devices[spi][i].pin == settings->cs.pin) break;
  }
  if (i == s_device_count[spi]) return STATUS_CODE_INVALID_ARGS;
  if (xSemaphoreTake(s_transaction_handle[spi], pdMS_TO_TICKS(1000U)) != pdTRUE) return STATUS_CODE_TIMEOUT;
  StatusCode result = STATUS_CODE_OK;
  for (i = 0; i < s_device_count[spi]; ++i) {
    result = gpio_set_state(&s_devices[spi][i], GPIO_STATE_HIGH);
    if (result != STATUS_CODE_OK) goto fail;
  }
  s_selected[spi] = settings->cs;
  if (select) result = gpio_set_state(&settings->cs, GPIO_STATE_LOW);
  if (result != STATUS_CODE_OK) goto fail;
  s_transaction_owner[spi] = xTaskGetCurrentTaskHandle();
  return STATUS_CODE_OK;
fail:
  xSemaphoreGive(s_transaction_handle[spi]);
  return result;
}
StatusCode spi_transaction_begin(SpiPort spi, const SpiSettings *settings) {
  return s_host_begin(spi, settings, true);
}
StatusCode spi_transaction_transfer(SpiPort spi, const uint8_t *tx, uint8_t *rx, size_t length, uint8_t filler, uint32_t timeout_ms) {
  if ((unsigned)spi >= NUM_SPI_PORTS || !length || !timeout_ms) return STATUS_CODE_INVALID_ARGS;
  if (!s_transaction_owner[spi] || s_transaction_owner[spi] != xTaskGetCurrentTaskHandle()) return STATUS_CODE_UNINITIALIZED;
  for (size_t i = 0; i < length; ++i) {
    uint8_t output = tx ? tx[i] : filler, input;
    status_ok_or_return(spi_write(spi, &output, 1U));
    status_ok_or_return(spi_read(spi, &input, 1U));
    if (rx) rx[i] = input;
  }
  return STATUS_CODE_OK;
}
StatusCode spi_transaction_end(SpiPort spi, bool trailing) {
  if ((unsigned)spi >= NUM_SPI_PORTS) return STATUS_CODE_INVALID_ARGS;
  if (!s_transaction_owner[spi] || s_transaction_owner[spi] != xTaskGetCurrentTaskHandle()) return STATUS_CODE_UNINITIALIZED;
  StatusCode result = gpio_set_state(&s_selected[spi], GPIO_STATE_HIGH);
  if (trailing && result == STATUS_CODE_OK) result = spi_transaction_transfer(spi, NULL, NULL, 1U, 0xFFU, 100U);
  s_transaction_owner[spi] = NULL;
  xSemaphoreGive(s_transaction_handle[spi]);
  return result;
}
StatusCode spi_deselected_clocks(SpiPort spi, const SpiSettings *settings, size_t length) {
  status_ok_or_return(s_host_begin(spi, settings, false));
  StatusCode result = spi_transaction_transfer(spi, NULL, NULL, length, 0xFFU, 100U);
  StatusCode end = spi_transaction_end(spi, false);
  return result != STATUS_CODE_OK ? result : end;
}
