/************************************************************************************************
 * @file   fota.c
 *
 * @brief  Source file for fota
 *
 * @date   2025-03-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_encryption.h"
#include "fota_error.h"
#include "packet_manger.h"

static PacketManager packet_manager;

static void datagram_complete_callback(FotaDatagram *datagram) {
  // TODO handle callback
}

FotaError fota_init(NetworkBuffer *network_buffer) {
  fota_encryption_init();

  packet_manager_init(&packet_manager, network_buffer, datagram_complete_callback);

  return FOTA_ERROR_SUCCESS;
}

void fota_process(void) {
  packet_manager_process(&packet_manager);
}