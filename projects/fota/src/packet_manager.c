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
  if (manager == NULL || network_buffer == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  manager->network_buffer = *network_buffer;
  manager->rx_state = PKT_STATE_WAITING_SOF;
  manager->bytes_received = 0U;
  memset(manager->current_packet, 0U, sizeof(FotaPacket));
  
  for (uint8_t i = 0; i < sizeof(manager->rx_packet_buffer); i++) {
    manager->rx_packet_buffer[i] = 0;
  }

  for (uint8_t i = 0; i < sizeof(manager->tx_packet_buffer); i++) {
    manager->tx_packet_buffer[i] = 0;
  }

  for (uint8_t i = 0; i < FOTA_MAX_ACTIVE_DATAGRAMS; i++) {
    manager->datagram_active[i] = false;
  }

  manager->datagram_complete_callback = callback;

  return FOTA_ERROR_SUCCESS;
}

FotaError packet_manager_process(PacketManager *manager) {
  if (manager == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  while(!network_buffer_empty(&manager->network_buffer)) {
    uint8_t byte;
    FotaError err = network_buffer_read(&manager->network_buffer, &byte);
    if (err != FOTA_ERROR_SUCCESS) {
      return err;
    }

    switch (manager->rx_state) {
      case PKT_STATE_WAITING_SOF:
        // check if SOF is byte
        if (byte == FOTA_PACKET_SOF) {
          manager->rx_packet_buffer[0] = byte;
          manager->bytes_received = 1;
          manager->rx_state = PKT_STATE_READING_HEADER;
        }
        break;

      case PKT_STATE_READING_HEADER:
        manager->bytes_received++;
        manager->rx_packet_buffer[manager->bytes_received] = byte;
        manager->rx_state = PKT_STATE_READING_PAYLOAD;
        break;

      case PKT_STATE_READING_PAYLOAD:
        manager->bytes_received++;
        manager->rx_packet_buffer[manager->bytes_received] = byte;

        if (manager->bytes_received == 258) {
          manager->rx_state = PKT_STATE_READING_EOF;
          
        }
        break;
      
      default:
        // reset
        manager->rx_state = PKT_STATE_WAITING_SOF;
        manager->bytes_received = 0;
        break;
    }
    

  }


  return FOTA_ERROR_SUCCESS;
}

FotaError packet_manager_send_datagram(PacketManager *manager, FotaDatagram *datagram, FotaError (*send_func)(int8_t *data, uint32_t length)) {
  if (manager == NULL || datagram == NULL || send_func == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  FotaPacket packets[FOTA_MAX_PACKETS_PER_DATAGRAM];
  uint16_t packet_num = 0;

  FotaError err = fota_datagram_to_packets(datagram, packets, &packet_num, FOTA_MAX_PACKETS_PER_DATAGRAM);
  if (err != FOTA_ERROR_SUCCESS) {
    return err;
  }

  for (uint16_t i = 0; i < packet_num; i++) { 
    // fota serialize
    uint8_t serialized_packet[FOTA_PACKET_MINIMUM_SIZE + FOTA_PACKET_PAYLOAD_SIZE];
    uint32_t bytes_written = 0;

    err = fota_packet_serialize(&packets[i], serialized_packet, sizeof(serialized_packet), &bytes_written);
    if (err != FOTA_ERROR_SUCCESS) {
      return err;
    }
    
    // network send
    err = send_func((int8_t *)serialized_packet, bytes_written);
    if (err != FOTA_ERROR_SUCCESS) {
      return err;
    }
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError packet_manager_create_datagram(PacketManager *manager, FotaDatagramType type, uint8_t *data, uint32_t length, FotaDatagram *datagram) {
  if (manager == NULL || datagram == NULL || (data == NULL && length > 0)) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  static uint32_t datagram_id = 0U;

  for (uint8_t i = 0U; i < FOTA_MAX_ACTIVE_DATAGRAMS; i++) {
    if (manager->datagram_active[i] == false) {
      datagram = &manager->active_datagrams[i];

      FotaError err = fota_datagram_init(datagram, type, datagram_id, data, length);
      if (err != FOTA_ERROR_SUCCESS) {
        return err;
      }

      manager->datagram_active[i] = true;
      datagram_id++;

      return FOTA_ERROR_SUCCESS; // create a 1 datagram 
    }
  }
  

  return FOTA_ERROR_NO_MEMORY;
}

FotaError packet_manager_get_datagram(PacketManager *manager, uint32_t datagram_id, FotaDatagram **datagram) {
  if (manager == NULL || datagram == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  for (uint8_t i = 0U; i < FOTA_MAX_ACTIVE_DATAGRAMS; i++) {
    if (manager->datagram_active[i] 
        && manager->active_datagrams[i].header.datagram_id == datagram_id) {
          datagram = &manager->active_datagrams[i];

          return FOTA_ERROR_SUCCESS;
        }
  }

  return FOTA_ERROR_NO_DATAGRAM_FOUND;
}

FotaError packet_manager_free_datagram(PacketManager *manager, uint32_t datagram_id) {
  if (manager == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  for (uint8_t i = 0U; i < FOTA_MAX_ACTIVE_DATAGRAMS; i++) {
    if (manager->datagram_active[i]
        && manager->active_datagrams[i].header.datagram_id == datagram_id) {
          
          manager->datagram_active[i] = false;
          manager->active_datagrams[i].is_complete = false;
          manager->active_datagrams[i].packets_received = 0U;

          for (uint32_t j = 0; j < FOTA_MAX_DATAGRAM_SIZE; j++) {
            manager->active_datagrams[i].data[j] = 0U;
          }

          for (uint32_t j = 0; j < FOTA_MAX_PACKETS_PER_DATAGRAM; j++) {
            manager->active_datagrams[i].packet_received[j] = false;
          }

          return FOTA_ERROR_SUCCESS;
        }
  }

  return FOTA_ERROR_NO_DATAGRAM_FOUND;
}
