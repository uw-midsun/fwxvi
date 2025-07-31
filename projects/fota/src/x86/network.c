/************************************************************************************************
 * @file   network.c
 *
 * @brief  Tx Rx UART implementation for firmware over the air (FOTA) updates
 *
 * @date   2025-07-05
 * @author Midnight Sun Team
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Inter-component Headers */
#include "network.h"
#include "network_buffer.h"

/* Intra-component Headers */

#define TX_SIM_BUFFER_SIZE 1024U

static NetworkBuffer *s_network_buffer = NULL;
static uint8_t s_tx_sim_buffer[TX_SIM_BUFFER_SIZE];
static size_t s_tx_sim_head = 0U;
static size_t s_tx_sim_tail = 0U;
static size_t s_tx_sim_len = 0U;

bool is_network_timeout(bool is_tx) {
  return false;
}

FotaError network_init(UartPort uart, UartSettings *settings, NetworkBuffer *network_buffer) {
  (void)uart;
  (void)settings;

  s_network_buffer = network_buffer;
  s_tx_sim_len = 0U;

  return FOTA_ERROR_SUCCESS;
}

FotaError network_tx(UartPort uart, uint8_t *data, size_t len) {
  (void)uart;

  if (!data || len == 0U || len > TX_SIM_BUFFER_SIZE) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  for (size_t i = 0U; i < len; i++) {
    size_t next = (s_tx_sim_head + 1) % TX_SIM_BUFFER_SIZE;

    if (next == s_tx_sim_tail) {
      return FOTA_ERROR_RESOURCE_EXHAUSTED;
    }

    s_tx_sim_buffer[s_tx_sim_head] = data[i];
    s_tx_sim_head = next;
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError network_read(UartPort uart, uint8_t *data, size_t len) {
  (void)uart;

  if (!s_network_buffer || !data) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  for (size_t i = 0U; i < len; i++) {
    if (network_buffer_read(s_network_buffer, &data[i]) != FOTA_ERROR_SUCCESS) {
      return FOTA_ERROR_INTERNAL_ERROR;
    }
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError network_sim_set_rx_data(uint8_t *data, size_t len) {
  if (!s_network_buffer || !data || len == 0) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  for (size_t i = 0U; i < len; i++) {
    if (network_buffer_write(s_network_buffer, &data[i]) != FOTA_ERROR_SUCCESS) {
      return FOTA_ERROR_INTERNAL_ERROR;
    }
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError network_sim_get_tx_data(uint8_t *data) {
  if (s_tx_sim_tail == s_tx_sim_head) {
    return FOTA_ERROR_RESOURCE_EXHAUSTED;
  }

  if (!data) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  *data = s_tx_sim_buffer[s_tx_sim_tail];
  s_tx_sim_tail = (s_tx_sim_tail + 1U) % TX_SIM_BUFFER_SIZE;

  return FOTA_ERROR_SUCCESS;
}
