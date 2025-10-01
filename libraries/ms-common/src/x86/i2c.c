/************************************************************************************************
 * @file    i2c.c
 *
 * @brief   I2C Library Source Code
 *
 * @date    2025-07-04
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "i2c.h"
#include "log.h"
#include "ms_semaphore.h"
#include "queues.h"
#include "status.h"

typedef enum I2CMode {
  I2C_MODE_TRANSMIT = 0,
  I2C_MODE_RECEIVE,
  NUM_I2C_MODES,
} I2CMode;

typedef struct {
  uint8_t buf[I2C_MAX_NUM_DATA];
  Queue queue;
  Mutex mutex;
} I2CBuffer;

typedef struct {
  I2CSettings settings;
  I2CBuffer i2c_tx_buf;
  I2CBuffer i2c_rx_buf;
  I2CMode curr_mode;
  I2CAddress current_addr;
  volatile uint8_t num_rx_bytes;
} I2CPortData;

static I2CPortData s_port[NUM_I2C_PORTS] = {
  [I2C_PORT_1] = {},
  [I2C_PORT_2] = {},
};

static StatusCode s_init_buffer(I2CBuffer *buf) {
  buf->queue.num_items = I2C_MAX_NUM_DATA;
  buf->queue.item_size = sizeof(uint8_t);
  buf->queue.storage_buf = buf->buf;
  return queue_init(&buf->queue);
}

StatusCode i2c_init(I2CPort i2c, const I2CSettings *settings) {
  if (i2c >= NUM_I2C_PORTS || settings->speed >= NUM_I2C_SPEEDS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_port[i2c].settings = *settings;

  status_ok_or_return(s_init_buffer(&s_port[i2c].i2c_tx_buf));
  status_ok_or_return(s_init_buffer(&s_port[i2c].i2c_rx_buf));

  return STATUS_CODE_OK;
}

StatusCode i2c_read(I2CPort i2c, I2CAddress addr, uint8_t *rx_data, size_t rx_len) {
  if (i2c >= NUM_I2C_PORTS) return STATUS_CODE_INVALID_ARGS;

  s_port[i2c].num_rx_bytes = rx_len;
  s_port[i2c].current_addr = addr;
  s_port[i2c].curr_mode = I2C_MODE_RECEIVE;

  for (size_t i = 0; i < rx_len; i++) {
    if (queue_receive(&s_port[i2c].i2c_rx_buf.queue, &rx_data[i], 0)) {
      queue_reset(&s_port[i2c].i2c_rx_buf.queue);
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }
  return STATUS_CODE_OK;
}

StatusCode i2c_write(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len) {
  if (i2c >= NUM_I2C_PORTS) return STATUS_CODE_INVALID_ARGS;

  for (size_t i = 0; i < tx_len; i++) {
    if (queue_send(&s_port[i2c].i2c_tx_buf.queue, &tx_data[i], 0)) {
      queue_reset(&s_port[i2c].i2c_tx_buf.queue);
      return STATUS_CODE_RESOURCE_EXHAUSTED;
    }
  }

  s_port[i2c].current_addr = addr;
  s_port[i2c].curr_mode = I2C_MODE_TRANSMIT;

  return STATUS_CODE_OK;
}

StatusCode i2c_read_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data, size_t rx_len) {
  if (i2c >= NUM_I2C_PORTS) return STATUS_CODE_INVALID_ARGS;

  status_ok_or_return(i2c_write(i2c, addr, &reg, 1));
  return i2c_read(i2c, addr, rx_data, rx_len);
}

StatusCode i2c_write_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data, size_t tx_len) {
  if (i2c >= NUM_I2C_PORTS) return STATUS_CODE_INVALID_ARGS;

  status_ok_or_return(i2c_write(i2c, addr, &reg, 1));
  return i2c_write(i2c, addr, tx_data, tx_len);
}

StatusCode i2c_set_rx_data(I2CPort i2c, uint8_t *rx_data, size_t rx_len) {
  if (i2c >= NUM_I2C_PORTS) return STATUS_CODE_INVALID_ARGS;

  for (size_t i = 0; i < rx_len; i++) {
    if (queue_send(&s_port[i2c].i2c_rx_buf.queue, &rx_data[i], 0)) {
      queue_reset(&s_port[i2c].i2c_rx_buf.queue);
      return STATUS_CODE_RESOURCE_EXHAUSTED;
    }
  }
  return STATUS_CODE_OK;
}

StatusCode i2c_get_tx_data(I2CPort i2c, uint8_t *tx_data, size_t tx_len) {
  if (i2c >= NUM_I2C_PORTS) return STATUS_CODE_INVALID_ARGS;

  for (size_t i = 0; i < tx_len; i++) {
    if (queue_receive(&s_port[i2c].i2c_tx_buf.queue, &tx_data[i], 0)) {
      queue_reset(&s_port[i2c].i2c_tx_buf.queue);
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }
  return STATUS_CODE_OK;
}

StatusCode i2c_read_mem(I2CPort i2c, I2CAddress addr, uint8_t mem_addr, uint8_t *rx_data, size_t rx_len) {
  return i2c_read_reg(i2c, addr, mem_addr, rx_data, rx_len);
}
