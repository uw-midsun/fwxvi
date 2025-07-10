/************************************************************************************************
 * @file    fota_datagram.c
 *
 * @brief   Fota Datagram Source File
 *
 * @date    2025-04-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_datagram.h"
#include "fota_encryption.h"

#define FOTA_DATAGRAM_HEADER_TOTAL_LENGTH_INDEX 0U
#define FOTA_DATAGRAM_HEADER_NUM_PACKETS_INDEX 4U
#define FOTA_DATAGRAM_HEADER_NODE_ID_INDEX 6U
#define FOTA_DATAGRAM_HEADER_DATAGRAM_CRC32_INDEX 8U
#define FOTA_DATAGRAM_BYTE_MASK 0xFFU
#define FOTA_DATAGRAM_WORD_SIZE 4U

FotaError fota_datagram_init(FotaDatagram *datagram, FotaDatagramType type, uint32_t datagram_id, uint8_t *data, uint32_t length) {
  if (datagram == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  datagram->header.type = type;
  datagram->header.datagram_id = datagram_id;
  datagram->header.total_length = length;
  datagram->header.num_packets = (length + FOTA_PACKET_PAYLOAD_SIZE - 1U) / FOTA_PACKET_PAYLOAD_SIZE;
  datagram->packets_received = 0U;
  datagram->is_complete = false;

  memset(datagram->packet_received, 0, sizeof(datagram->packet_received));

  if (data != NULL && length > 0) {
    memcpy(datagram->data, data, length);

    if (length % FOTA_DATAGRAM_WORD_SIZE != 0U) {
      length += FOTA_DATAGRAM_WORD_SIZE - (length % FOTA_DATAGRAM_WORD_SIZE);
    }

    datagram->header.datagram_crc32 = fota_calculate_crc32(data, length / FOTA_DATAGRAM_WORD_SIZE);
  } else {
    datagram->header.datagram_crc32 = 0U;
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_datagram_to_packets(FotaDatagram *datagram, FotaPacket *packets, uint16_t *num_packets, uint16_t max_packets) {
  if (datagram == NULL || packets == NULL || num_packets == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  if (datagram->header.num_packets + 1U > max_packets) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  /* Header packet */
  FotaPacket *header = &packets[0];
  fota_error_ok_or_return(fota_packet_init(header, FOTA_PACKET_TYPE_HEADER, 0U, FOTA_DATAGRAM_HEADER_PAYLOAD_SIZE));

  /* Total length encoded in little-endian */
  header->payload[FOTA_DATAGRAM_HEADER_TOTAL_LENGTH_INDEX] = (uint8_t)(datagram->header.total_length & FOTA_DATAGRAM_BYTE_MASK);
  header->payload[FOTA_DATAGRAM_HEADER_TOTAL_LENGTH_INDEX + 1U] = (uint8_t)((datagram->header.total_length >> 8U) & FOTA_DATAGRAM_BYTE_MASK);
  header->payload[FOTA_DATAGRAM_HEADER_TOTAL_LENGTH_INDEX + 2U] = (uint8_t)((datagram->header.total_length >> 16U) & FOTA_DATAGRAM_BYTE_MASK);
  header->payload[FOTA_DATAGRAM_HEADER_TOTAL_LENGTH_INDEX + 3U] = (uint8_t)((datagram->header.total_length >> 24U) & FOTA_DATAGRAM_BYTE_MASK);

  /* Num packets encoded in little-endian */
  header->payload[FOTA_DATAGRAM_HEADER_NUM_PACKETS_INDEX] = (uint8_t)(datagram->header.num_packets & FOTA_DATAGRAM_BYTE_MASK);
  header->payload[FOTA_DATAGRAM_HEADER_NUM_PACKETS_INDEX + 1U] = (uint8_t)((datagram->header.num_packets >> 8U) & FOTA_DATAGRAM_BYTE_MASK);

  /* Target node ID encoded in little-endian */
  header->payload[FOTA_DATAGRAM_HEADER_NODE_ID_INDEX] = (uint8_t)(datagram->header.target_node_id & FOTA_DATAGRAM_BYTE_MASK);
  header->payload[FOTA_DATAGRAM_HEADER_NODE_ID_INDEX + 1U] = (uint8_t)((datagram->header.target_node_id >> 8U) & FOTA_DATAGRAM_BYTE_MASK);

  /* Datagram CRC32 stored in little-endian */
  header->payload[FOTA_DATAGRAM_HEADER_DATAGRAM_CRC32_INDEX] = (uint8_t)(datagram->header.datagram_crc32 & FOTA_DATAGRAM_BYTE_MASK);
  header->payload[FOTA_DATAGRAM_HEADER_DATAGRAM_CRC32_INDEX + 1U] = (uint8_t)((datagram->header.datagram_crc32 >> 8U) & FOTA_DATAGRAM_BYTE_MASK);
  header->payload[FOTA_DATAGRAM_HEADER_DATAGRAM_CRC32_INDEX + 2U] = (uint8_t)((datagram->header.datagram_crc32 >> 16U) & FOTA_DATAGRAM_BYTE_MASK);
  header->payload[FOTA_DATAGRAM_HEADER_DATAGRAM_CRC32_INDEX + 3U] = (uint8_t)((datagram->header.datagram_crc32 >> 24U) & FOTA_DATAGRAM_BYTE_MASK);

  header->datagram_id = datagram->header.datagram_id;
  fota_packet_set_crc(header);

  /* Data packets */
  for (uint16_t i = 0U; i < datagram->header.num_packets; ++i) {
    FotaPacket *cur = &packets[i + 1U];
    uint32_t offset = i * FOTA_PACKET_PAYLOAD_SIZE;
    uint32_t remaining = datagram->header.total_length - offset;
    uint16_t payload_len = (remaining < FOTA_PACKET_PAYLOAD_SIZE) ? (uint16_t)remaining : FOTA_PACKET_PAYLOAD_SIZE;

    fota_error_ok_or_return(fota_packet_init(cur, FOTA_PACKET_TYPE_DATA, i, payload_len));
    cur->datagram_id = datagram->header.datagram_id;
    memcpy(cur->payload, &datagram->data[offset], payload_len);
    fota_error_ok_or_return(fota_packet_set_crc(cur));
  }

  *num_packets = datagram->header.num_packets + 1U;

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_datagram_process_data_packet(FotaDatagram *datagram, FotaPacket *packet) {
  if (datagram == NULL || packet == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  fota_error_ok_or_return(fota_verify_packet_encryption(packet));

  if (datagram->header.datagram_id != packet->datagram_id) {
    return FOTA_ERROR_INTERNAL_ERROR;
  }

  if (packet->sequence_num >= FOTA_MAX_PACKETS_PER_DATAGRAM || datagram->packet_received[packet->sequence_num]) {
    return FOTA_ERROR_INTERNAL_ERROR;
  }

  datagram->packet_received[packet->sequence_num] = true;
  datagram->packets_received++;

  for (uint16_t i = 0U; i < packet->payload_length; ++i) {
    datagram->data[packet->sequence_num * FOTA_PACKET_PAYLOAD_SIZE + i] = packet->payload[i];
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_datagram_process_header_packet(FotaDatagram *datagram, FotaPacket *packet) {
  if (datagram == NULL || packet == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  fota_error_ok_or_return(fota_verify_packet_encryption(packet));

  datagram->header.datagram_crc32 =
      (((uint32_t)packet->payload[FOTA_DATAGRAM_HEADER_DATAGRAM_CRC32_INDEX + 3U] << 24U) | ((uint32_t)packet->payload[FOTA_DATAGRAM_HEADER_DATAGRAM_CRC32_INDEX + 2U] << 16U) |
       ((uint32_t)packet->payload[FOTA_DATAGRAM_HEADER_DATAGRAM_CRC32_INDEX + 1U] << 8U) | ((uint32_t)packet->payload[FOTA_DATAGRAM_HEADER_DATAGRAM_CRC32_INDEX]));

  datagram->header.packet_crc32 = packet->crc32;
  datagram->header.datagram_id = packet->datagram_id;

  datagram->header.total_length = (((uint32_t)packet->payload[FOTA_DATAGRAM_HEADER_TOTAL_LENGTH_INDEX + 3U] << 24U) | ((uint32_t)packet->payload[FOTA_DATAGRAM_HEADER_TOTAL_LENGTH_INDEX + 2U] << 16U) |
                                   ((uint32_t)packet->payload[FOTA_DATAGRAM_HEADER_TOTAL_LENGTH_INDEX + 1U] << 8U) | ((uint32_t)packet->payload[FOTA_DATAGRAM_HEADER_TOTAL_LENGTH_INDEX]));

  datagram->header.num_packets = ((uint16_t)packet->payload[FOTA_DATAGRAM_HEADER_NUM_PACKETS_INDEX + 1U] << 8U) | (uint16_t)packet->payload[FOTA_DATAGRAM_HEADER_NUM_PACKETS_INDEX];
  datagram->header.target_node_id = ((uint16_t)packet->payload[FOTA_DATAGRAM_HEADER_NODE_ID_INDEX + 1U] << 8U) | (uint16_t)packet->payload[FOTA_DATAGRAM_HEADER_NODE_ID_INDEX];

  datagram->packets_received = 0U;
  datagram->is_complete = false;
  memset(datagram->packet_received, 0U, sizeof(datagram->packet_received));

  return FOTA_ERROR_SUCCESS;
}

bool fota_datagram_is_complete(FotaDatagram *datagram) {
  if (datagram == NULL) {
    return false;
  }

  datagram->is_complete = (datagram->packets_received == datagram->header.num_packets);
  return datagram->is_complete;
}

FotaError fota_datagram_verify(FotaDatagram *datagram) {
  if (datagram == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  return fota_verify_datagram_encryption(datagram);
}
