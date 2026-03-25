/************************************************************************************************
 * @file   fota_packet.c
 *
 * @brief  Packet implementation for firmware over the air (FOTA) updates
 *
 * @date   2025-04-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <string.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_encryption.h"
#include "fota_packet.h"

FotaError fota_packet_init(FotaPacket *packet, FotaPacketType type, uint8_t sequence, uint16_t length) {
  if (packet == NULL || length > FOTA_PACKET_PAYLOAD_SIZE) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  packet->sof = FOTA_PACKET_SOF;
  packet->packet_type = type;
  packet->datagram_id = 0U;
  packet->sequence_num = sequence;
  packet->payload_length = length;
  memset(packet->payload, 0U, FOTA_PACKET_PAYLOAD_SIZE);
  packet->crc32 = 0U;
  packet->eof = FOTA_PACKET_EOF;

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_packet_set_crc(FotaPacket *packet) {
  if (packet == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  uint8_t *crc32_data_start = (uint8_t *)packet->payload;
  uint32_t crc32_data_size = packet->payload_length;

  /* Handle padding */
  if (crc32_data_size % 4U != 0U) {
    crc32_data_size += crc32_data_size % 4U;
  }

  crc32_data_size /= 4U; /* Convert bytes to words */

  packet->crc32 = fota_calculate_crc32(crc32_data_start, crc32_data_size);
  return FOTA_ERROR_SUCCESS;
}

FotaError fota_packet_serialize(FotaPacket *packet, uint8_t *buffer, uint32_t buf_size, uint32_t *bytes_written) {
  if (packet == NULL || buffer == NULL || bytes_written == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  if (buf_size < FOTA_PACKET_PAYLOAD_SIZE) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  uint8_t *ptr = buffer;
  *bytes_written = 0U;

  *(ptr++) = packet->sof;
  (*bytes_written)++;

  *(ptr++) = packet->packet_type;
  (*bytes_written)++;

  *((uint32_t *)ptr) = packet->datagram_id;
  ptr += sizeof(packet->datagram_id);
  *bytes_written += sizeof(packet->datagram_id);

  *(ptr++) = packet->sequence_num;
  (*bytes_written)++;

  *((uint16_t *)ptr) = packet->payload_length;
  ptr += sizeof(packet->payload_length);
  *bytes_written += sizeof(packet->payload_length);

  memcpy(ptr, packet->payload, FOTA_PACKET_PAYLOAD_SIZE);
  ptr += FOTA_PACKET_PAYLOAD_SIZE;
  *bytes_written += FOTA_PACKET_PAYLOAD_SIZE;

  *((uint32_t *)ptr) = packet->crc32;
  ptr += sizeof(packet->crc32);
  *bytes_written += sizeof(packet->crc32);

  *(ptr++) = packet->eof;
  (*bytes_written)++;

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_packet_deserialize(FotaPacket *packet, uint8_t *buffer, uint32_t buf_size) {
  if (packet == NULL || buffer == NULL || buf_size < FOTA_PACKET_PAYLOAD_SIZE) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  uint8_t *ptr = buffer;

  packet->sof = *(ptr++);
  if (packet->sof != FOTA_PACKET_SOF) {
    return FOTA_ERROR_INVALID_PACKET;
  }

  packet->packet_type = *(ptr++);

  packet->datagram_id = *((uint32_t *)ptr);
  ptr += sizeof(packet->datagram_id);

  packet->sequence_num = *(ptr++);

  packet->payload_length = *((uint16_t *)ptr);
  ptr += sizeof(packet->payload_length);

  if (packet->payload_length > FOTA_PACKET_PAYLOAD_SIZE) {
    return FOTA_ERROR_INVALID_PACKET;
  }

  memcpy(packet->payload, ptr, FOTA_PACKET_PAYLOAD_SIZE);
  ptr += FOTA_PACKET_PAYLOAD_SIZE;

  packet->crc32 = *((uint32_t *)ptr);
  ptr += sizeof(packet->crc32);

  packet->eof = *(ptr++);
  if (packet->eof != FOTA_PACKET_EOF) {
    return FOTA_ERROR_INVALID_PACKET;
  }

  return FOTA_ERROR_SUCCESS;
}
