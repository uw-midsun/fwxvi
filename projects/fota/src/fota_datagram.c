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
  return FOTA_ERROR_SUCCESS;
}

FotaError fota_datagram_to_packets(FotaDatagram *datagram, FotaPacket *packets, uint16_t *num_packets, uint16_t max_packets) {
  return FOTA_ERROR_SUCCESS;
}

FotaError fota_datagram_process_packet(FotaDatagram *datagram, FotaPacket *packet) {
  if (fota_verify_packet_encryption(packet) != packet->crc32) return FOTA_ERROR_INTERNAL_ERROR;
  if ((datagram->header).datagram_id != packet->datagram_id) return FOTA_ERROR_INTERNAL_ERROR;
  if (datagram->packet_received[packet->sequence_num]) return FOTA_ERROR_INTERNAL_ERROR; 

  ++(datagram->packets_received); 
  datagram->packet_received[packet->sequence_num] = true; 

  for (int i = 0; i < packet->payload_length; ++i) {
        datagram->data[packet->sequence_num * 256 + i] = packet->payload[i]; 
  }
}

bool fota_datagram_is_complete(FotaDatagram *datagram) {
  return datagram->is_complete = (datagram->packets_received == (datagram->header).num_packets);
}

FotaError fota_datagram_verify(FotaDatagram *datagram) {
  return fota_verify_datagram_encryption(datagram); 
}
