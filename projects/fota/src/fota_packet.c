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
  if (packet == NULL || length > FOTA_PACKET_PAYLOAD_SIZE || length % 4U != 0U) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  packet->sof = FOTA_PACKET_SOF;
  packet->packet_type = type;
  packet->datagram_id = 0U; /* To be set by the caller */
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

  uint32_t payload_word_size = packet->payload_length / 4U;
  packet->crc32 = fota_calculate_crc32(packet->payload, payload_word_size);

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_packet_serialize(FotaPacket *packet, uint8_t *buffer, uint32_t buf_size, uint32_t *bytes_written) {
  if (packet == NULL || buffer == NULL || bytes_written == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  uint32_t total_size = sizeof(packet->sof) + sizeof(packet->packet_type) + sizeof(packet->datagram_id) + sizeof(packet->sequence_num) + sizeof(packet->payload_length) + packet->payload_length +
                        sizeof(packet->crc32) + sizeof(packet->eof);

  if (buf_size < total_size) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  uint8_t *buffer_pointer = buffer;

  *(buffer_pointer++) = packet->sof;
  *(buffer_pointer++) = packet->packet_type;

  *((uint32_t *)buffer_pointer) = packet->datagram_id;
  buffer_pointer += sizeof(packet->datagram_id);

  *(buffer_pointer++) = packet->sequence_num;

  *((uint16_t *)buffer_pointer) = packet->payload_length;
  buffer_pointer += sizeof(packet->payload_length);

  for (uint16_t i = 0U; i < packet->payload_length; ++i) {
    *(buffer_pointer++) = packet->payload[i];
  }

  *((uint32_t *)buffer_pointer) = packet->crc32;
  buffer_pointer += sizeof(packet->crc32);

  *(buffer_pointer++) = packet->eof;
  *bytes_written = total_size;

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_packet_deserialize(FotaPacket *packet, uint8_t *buffer, uint32_t buf_size) {
  if (packet == NULL || buffer == NULL || buf_size < FOTA_PACKET_MINIMUM_SIZE) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  uint8_t *buffer_pointer = buffer;

  packet->sof = *(buffer_pointer++);
  packet->packet_type = *(buffer_pointer++);

  packet->datagram_id = *((uint32_t *)buffer_pointer);
  buffer_pointer += sizeof(packet->datagram_id);

  packet->sequence_num = *(buffer_pointer++);

  packet->payload_length = *((uint16_t *)buffer_pointer);
  buffer_pointer += sizeof(packet->payload_length);

  if (packet->payload_length > FOTA_PACKET_PAYLOAD_SIZE || packet->payload_length % 4U != 0U) {
    return FOTA_ERROR_INVALID_PACKET;
  }

  for (uint16_t i = 0U; i < packet->payload_length; ++i) {
    packet->payload[i] = *(buffer_pointer++);
  }

  packet->crc32 = *((uint32_t *)buffer_pointer);
  buffer_pointer += sizeof(packet->crc32);

  packet->eof = *(buffer_pointer++);

  if (packet->sof != 0xAA || packet->eof != 0xBB) {
    return FOTA_ERROR_INVALID_PACKET;
  }

  return FOTA_ERROR_SUCCESS;
}
