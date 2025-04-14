/************************************************************************************************
 * @file   fota_packet.c
 *
 * @brief  Packet implementation for firmware over the air (FOTA) updates
 *
 * @date   2025-04-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_packet.h"

#include "network_buffer.h"

void packet_init(Packet *packet, PacketType type, uint8_t sequence, uint16_t length, uint8_t SOF, uint8_t EOF) {
  if (!packet) return;
  packet->sof = SOF;
  packet->packet_type = type;
  packet->sequence_num = sequence;
  packet->payload_length = length;
  memset(packet->payload.raw_payload, 0, PAYLOAD_SIZE);
  packet->eof = EOF;
}

FotaError encode_packet(const Packet *packet, NetworkBuffer *buffer) {
  if (!packet || !buffer) {
    return FOTA_ERROR_INVALID_ARGS;
  }
  network_buffer_init(buffer);
  FotaError error = network_buffer_write(buffer, &packet->sof);
  if (error != FOTA_ERROR_SUCCESS) return error;
  error = network_buffer_write(buffer, &packet->eof);
  if (error != FOTA_ERROR_SUCCESS) return error;
  uint8_t len_lo = packet->payload_length & 0xFF;
  uint8_t len_hi = (packet->payload_length >> 8) & 0xFF;
  error = network_buffer_write(buffer, &len_lo);
  if (error != FOTA_ERROR_SUCCESS) return error;
  error = network_buffer_write(buffer, &len_hi);
  if (error != FOTA_ERROR_SUCCESS) return error;
  error = network_buffer_write(buffer, &packet->sequence_num);
  if (error != FOTA_ERROR_SUCCESS) return error;
  if (packet->payload_length > PAYLOAD_SIZE) return FOTA_RESOURCE_EXHAUSTED;
}

FotaError decode_packet(Packet *packet, NetworkBuffer *net_buf) {
  if (!packet || !net_buf) {
    return FOTA_ERROR_INVALID_ARGS;
  }
  if (network_buffer_num_items(net_buf) < 6) {
    return FOTA_ERROR_INTERNAL_ERROR;
  }
  uint8_t sof = 0;
  if (network_buffer_read(net_buf, &sof) != FOTA_ERROR_SUCCESS || sof != SOF) {
    return FOTA_ERROR_INTERNAL_ERROR;
  }
  packet->sof = SOF;
  uint8_t type, seq;
  uint8_t length_bytes[2];
  if (network_buffer_read(net_buf, &type) != FOTA_ERROR_SUCCESS || network_buffer_read(net_buf, &seq) != FOTA_ERROR_SUCCESS || network_buffer_read(net_buf, &length_bytes[0]) != FOTA_ERROR_SUCCESS ||
      network_buffer_read(net_buf, &length_bytes[1]) != FOTA_ERROR_SUCCESS) {
    return FOTA_ERROR_INTERNAL_ERROR;
  }
  packet->packet_type = (PacketType)type;
  packet->sequence_num = seq;
  packet->payload_length = (length_bytes[1] << 8) | length_bytes[0];
  if (packet->payload_length > PAYLOAD_SIZE) return FOTA_RESOURCE_EXHAUSTED;

  for (uint16_t i = 0; i < packet->payload_length; ++i) {
    if (network_buffer_read(net_buf, &packet->payload[i]) != FOTA_ERROR_SUCCESS) {
      return FOTA_RESOURCE_EXHAUSTED;
    }
  }

  uint8_t eof;
  if (network_buffer_read(net_buf, &eof) != FOTA_ERROR_SUCCESS || eof != EOF) {
    return FOTA_RESOURCE_EXHAUSTED;
  }
  packet->eof = eof;
  if (packet->payload_length < 4) {
    return FOTA_ERROR_CRC_MISMATCH;
  }
  // At least the header size

  uint32_t received_crc = 0;
  memcpy(&received_crc, &packet->payload.raw_payload[packet->payload_length - 4], sizeof(uint32_t));

  uint32_t calculated_crc = crc32_calculate(&packet->payload.raw_payload, packet->payload_length - 4);
  if (received_crc != calculated_crc) {
    return FOTA_ERROR_CRC_MISMATCH;
  }

  return FOTA_ERROR_SUCCESS;
}
