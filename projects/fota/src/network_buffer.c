/************************************************************************************************
 * @file   network_buffer.c
 *
 * @brief  Circular Buffer class implementation for firmware over the air (FOTA) updates
 *
 * @date   2025-04-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "network_buffer.h"

FotaError network_buffer_init(NetworkBuffer *network_buffer) {
  if (network_buffer == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  memset(network_buffer->data, 0U, NETWORK_BUFFER_SIZE);
  network_buffer->num_items = 0U;
  network_buffer->write_index = 0U;
  network_buffer->read_index = 0U;

  return FOTA_ERROR_SUCCESS;
}

uint16_t network_buffer_num_items(NetworkBuffer *network_buffer) {
  return network_buffer->num_items;
}

uint16_t network_buffer_num_empty_slots(NetworkBuffer *network_buffer) {
  return NETWORK_BUFFER_SIZE - network_buffer->num_items;
}

bool network_buffer_empty(NetworkBuffer *network_buffer) {
  return network_buffer->num_items == 0U;
}

bool network_buffer_full(NetworkBuffer *network_buffer) {
  return network_buffer->num_items == NETWORK_BUFFER_SIZE;
}

FotaError network_buffer_write(NetworkBuffer *network_buffer, uint8_t *data) {
  if (network_buffer == NULL || data == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  if (network_buffer_full(network_buffer)) {
    return FOTA_ERROR_RESOURCE_EXHAUSTED;
  }

  network_buffer->data[network_buffer->write_index] = *data;

  network_buffer->write_index = (network_buffer->write_index + 1U) % NETWORK_BUFFER_SIZE;
  network_buffer->num_items++;

  return FOTA_ERROR_SUCCESS;
}

FotaError network_buffer_read(NetworkBuffer *network_buffer, uint8_t *data) {
  if (network_buffer == NULL || data == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  if (network_buffer_empty(network_buffer)) {
    return FOTA_ERROR_RESOURCE_EXHAUSTED;
  }

  *data = network_buffer->data[network_buffer->read_index];

  network_buffer->read_index = (network_buffer->read_index + 1U) % NETWORK_BUFFER_SIZE;
  network_buffer->num_items--;

  return FOTA_ERROR_SUCCESS;
}
