/************************************************************************************************
 * @file    fota_datagram.h
 *
 * @brief   Fota Datagram Source File
 *
 * @date    2025-04-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_datagram.h"

#include "fota_encryption.h"

FotaError fota_datagram_init(FotaDatagram *datagram, FotaDatagramType type, uint32_t datagram_id, uint8_t *data, uint32_t length) {
  datagram->header.type = type;
  datagram->header.datagram_id = datagram_id;
  datagram->header.total_length = length;
  datagram->header.num_packets = (length + FOTA_PACKET_PAYLOAD_SIZE - 1) / FOTA_PACKET_PAYLOAD_SIZE;

  if (data) {
    memcpy(datagram->data, data, length);
    datagram->header.crc32 = fota_calculate_crc32(data, length);
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_datagram_to_packets(FotaDatagram *datagram, FotaPacket *packets, uint16_t *num_packets, uint16_t max_packets) {
  if (datagram->header.num_packets > max_packets) return FOTA_ERROR_INVALID_ARGS;

  for (int i = 0; i < datagram->header.num_packets; ++i) {
    FotaPacket *cur = &packets[i];

    cur->datagram_id = datagram->header.datagram_id;
    cur->packet_type = FOTA_PACKET_TYPE_DATA;

    uint32_t remaining_length = datagram->header.total_length - i * FOTA_PACKET_PAYLOAD_SIZE;
    cur->payload_length = remaining_length < FOTA_PACKET_PAYLOAD_SIZE ? remaining_length : FOTA_PACKET_PAYLOAD_SIZE;

    cur->sequence_num = i;
    cur->sof = FOTA_PACKET_SOF;
    cur->eof = FOTA_PACKET_EOF;

    memcpy(cur->payload, &datagram->data[i * FOTA_PACKET_PAYLOAD_SIZE], remaining_length);

    cur->crc32 = fota_calculate_crc32(cur->payload, remaining_length);
  }

  *num_packets = datagram->header.num_packets;

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_datagram_process_packet(FotaDatagram *datagram, FotaPacket *packet) {
  if (fota_verify_packet_encryption(packet) != packet->crc32) return FOTA_ERROR_INTERNAL_ERROR;
  if ((datagram->header).datagram_id != packet->datagram_id) return FOTA_ERROR_INTERNAL_ERROR;
  if (datagram->packet_received[packet->sequence_num]) return FOTA_ERROR_INTERNAL_ERROR;

  ++(datagram->packets_received);
  datagram->packet_received[packet->sequence_num] = true;

  for (int i = 0; i < packet->payload_length; ++i) {
    datagram->data[packet->sequence_num * FOTA_PACKET_PAYLOAD_SIZE + i] = packet->payload[i];
  }

  return FOTA_ERROR_SUCCESS;
}

bool fota_datagram_is_complete(FotaDatagram *datagram) {
  return datagram->is_complete = (datagram->packets_received == (datagram->header).num_packets);
}

FotaError fota_datagram_verify(FotaDatagram *datagram) {
  return fota_verify_datagram_encryption(datagram);
}
