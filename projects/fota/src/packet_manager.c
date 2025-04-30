/************************************************************************************************
 * @file    packet_manager.c
 *
 * @brief   Packet Manager Source File
 *
 * @date    2025-04-29
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "packet_manger.h"

FotaError packet_manager_init(PacketManager *manager, NetworkBuffer *network_buffer, void (*callback)(FotaDatagram *datagram)) {
  return FOTA_ERROR_SUCCESS;
}

FotaError packet_manager_process(PacketManager *manager) {
  return FOTA_ERROR_SUCCESS;
}

FotaError packet_manager_send_datagram(PacketManager *manager, FotaDatagram *datagram, FotaError (*send_func)(int8_t *data, uint32_t length)) {
  return FOTA_ERROR_SUCCESS;
}

FotaError packet_manager_create_datagram(PacketManager *manager, FotaDatagramType type, uint8_t *data, uint32_t length, FotaDatagram *datagram) {
  return FOTA_ERROR_SUCCESS;
}

FotaError packet_manager_get_datagram(PacketManager *manager, uint32_t datagram_id, FotaDatagram **datagram) {
  return FOTA_ERROR_SUCCESS;
}

FotaError packet_manager_free_datagram(PacketManager *manager, uint32_t datagram_id) {
  return FOTA_ERROR_SUCCESS;
}
