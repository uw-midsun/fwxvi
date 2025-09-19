/************************************************************************************************
 * @file    fota_encryption.c
 *
 * @brief   Fota Encryption Source File
 *
 * @date    2025-04-29
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_encryption.h"
#include "fota_flash.h"

#define FOTA_ENCRYPTION_WORD_SIZE 4U

static uint32_t crc32_compute(uint8_t *data, uint32_t len_bytes) {
  uint32_t crc = 0xFFFFFFFFU;

  for (uint32_t i = 0U; i < len_bytes; ++i) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8U; ++j) {
      if (crc & 1U)
        crc = (crc >> 1) ^ 0xEDB88320U;
      else
        crc >>= 1;
    }
  }

  return ~crc;
}

FotaError fota_encryption_init() {
  return FOTA_ERROR_SUCCESS;
}

uint32_t fota_calculate_crc32(uint8_t *data_start, uint32_t word_size) {
  if (data_start == NULL) {
    return 0U;
  }

  return crc32_compute(data_start, word_size * FOTA_ENCRYPTION_WORD_SIZE);
}

uint32_t fota_calculate_crc32_on_flash_memory(uintptr_t flash_base_addr, uint32_t flash_size) {
  /* FOTA Flash layer will handle invalid addresses */
  return 0;
}

FotaError fota_verify_packet_encryption(FotaPacket *packet) {
  if (packet == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  uint32_t received_crc32 = packet->crc32;

  uint8_t *crc32_data_start = (uint8_t *)packet->payload;
  uint32_t crc32_data_size = packet->payload_length;

  /* Handle padding */
  if (crc32_data_size % FOTA_ENCRYPTION_WORD_SIZE != 0U) {
    crc32_data_size += FOTA_ENCRYPTION_WORD_SIZE - (crc32_data_size % FOTA_ENCRYPTION_WORD_SIZE);
  }

  crc32_data_size /= FOTA_ENCRYPTION_WORD_SIZE; /* Convert bytes to words */

  uint32_t computed_crc32 = fota_calculate_crc32(crc32_data_start, crc32_data_size);
  if (computed_crc32 != received_crc32) {
    return FOTA_ERROR_CRC32_MISMATCH;
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_verify_datagram_encryption(FotaDatagram *datagram) {
  if (datagram == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  uint32_t received_crc32 = datagram->header.datagram_crc32;

  uint8_t *crc32_data_start = (uint8_t *)datagram->data;
  uint32_t crc32_data_size = datagram->header.total_length;

  /* Handle padding */
  if (crc32_data_size % FOTA_ENCRYPTION_WORD_SIZE != 0U) {
    crc32_data_size += FOTA_ENCRYPTION_WORD_SIZE - (crc32_data_size % FOTA_ENCRYPTION_WORD_SIZE);
  }

  crc32_data_size /= FOTA_ENCRYPTION_WORD_SIZE; /* Convert bytes to words */

  uint32_t computed_crc32 = fota_calculate_crc32(crc32_data_start, crc32_data_size);

  if (computed_crc32 != received_crc32) {
    return FOTA_ERROR_CRC32_MISMATCH;
  }

  return FOTA_ERROR_SUCCESS;
}
