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

FotaError fota_datagram_init(FotaDatagram *datagram, FotaDatagramType type, uint32_t datagram_id, uint8_t *data, uint32_t length) {
  return FOTA_ERROR_SUCCESS;
}

FotaError fota_datagram_to_packets(FotaDatagram *datagram, FotaPacket *packets, uint16_t *num_packets, uint16_t max_packets) {
  return FOTA_ERROR_SUCCESS;
}

FotaError fota_datagram_process_packet(FotaDatagram *datagram, FotaPacket *packet) {
  return FOTA_ERROR_SUCCESS;
}

bool fota_datagram_is_complete(FotaDatagram *datagram) {
  return datagram->is_complete = (datagram->packets_received == datagram->header.num_packets);
}

FotaError fota_datagram_verify(FotaDatagram *datagram) {
  return FOTA_ERROR_SUCCESS;
}
