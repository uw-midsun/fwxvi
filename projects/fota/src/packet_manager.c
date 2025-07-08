/************************************************************************************************
 * @file    packet_manager.c
 *
 * @brief   Packet Manager implementation for FOTA updates
 *
 * @date    2025-04-29
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdio.h>
#include <string.h>

/* Intra-component Headers */
#include "fota_log.h"
#include "network_buffer.h"
#include "packet_manager.h"

/* Inter-component Headers */

static uint32_t s_next_datagram_id = 1U;

FotaError packet_manager_init(PacketManager *manager, UartPort uart, UartSettings *uart_settings, FotaDatagramCompleteCb callback) {
  if (manager == NULL || uart_settings == NULL || uart >= NUM_UART_PORTS) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  FotaError err = network_init(uart, uart_settings, &manager->network_buffer);
  if (err != FOTA_ERROR_SUCCESS) {
    return err;
  }

  manager->rx_state = PKT_STATE_WAITING_SOF;
  manager->bytes_received = 0U;
  memset(&manager->current_packet, 0U, sizeof(FotaPacket));
  memset(manager->rx_packet_buffer, 0U, sizeof(manager->rx_packet_buffer));
  memset(manager->tx_packet_buffer, 0U, sizeof(manager->tx_packet_buffer));
  memset(manager->datagram_active, 0U, sizeof(manager->datagram_active));
  manager->datagram_complete_callback = callback;

  return FOTA_ERROR_SUCCESS;
}

FotaError packet_manager_process(PacketManager *manager) {
  if (manager == NULL) return FOTA_ERROR_INVALID_ARGS;

  while (!network_buffer_empty(&manager->network_buffer)) {
    uint8_t byte;
    FotaError err = network_buffer_read(&manager->network_buffer, &byte);
    if (err != FOTA_ERROR_SUCCESS) return err;

    switch (manager->rx_state) {
      case PKT_STATE_WAITING_SOF:
        if (byte == FOTA_PACKET_SOF) {
          manager->rx_packet_buffer[0] = byte;
          manager->bytes_received = 1;
          manager->rx_state = PKT_STATE_READING_PACKET;
        }
        break;

      case PKT_STATE_READING_PACKET:
        manager->rx_packet_buffer[manager->bytes_received++] = byte;

        if (manager->bytes_received == FOTA_PACKET_SERIALIZED_SIZE) {
          err = fota_packet_deserialize(&manager->current_packet, manager->rx_packet_buffer, FOTA_PACKET_SERIALIZED_SIZE);
          if (err != FOTA_ERROR_SUCCESS) {
            LOG_FOTA_DEBUG("Deserialize failed: %u\r\n", err);
            break;
          }

          err = fota_verify_packet_encryption(&manager->current_packet);
          if (err != FOTA_ERROR_SUCCESS) {
            LOG_FOTA_DEBUG("Encryption failed: %u\r\n", err);
            break;
          }

          FotaDatagram *datagram = NULL;
          err = packet_manager_get_datagram(manager, manager->current_packet.datagram_id, &datagram);

          if (err == FOTA_ERROR_NO_DATAGRAM_FOUND && manager->current_packet.packet_type == FOTA_PACKET_TYPE_HEADER) {
            err = packet_manager_create_datagram(manager, FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK, NULL, 0, &datagram);
            if (err != FOTA_ERROR_SUCCESS) {
              LOG_FOTA_DEBUG("Create datagram failed: %u\r\n", err);
              break;
            }

            datagram->header.datagram_id = manager->current_packet.datagram_id;
          }

          if (datagram == NULL) {
            break;
          }

          if (manager->current_packet.packet_type == FOTA_PACKET_TYPE_HEADER) {
            err = fota_datagram_process_header_packet(datagram, &manager->current_packet);
          } else {
            err = fota_datagram_process_data_packet(datagram, &manager->current_packet);
          }

          if (err == FOTA_ERROR_SUCCESS && fota_datagram_is_complete(datagram)) {
            if (fota_datagram_verify(datagram) == FOTA_ERROR_SUCCESS && manager->datagram_complete_callback != NULL) {
              manager->datagram_complete_callback(datagram);
            }
            packet_manager_free_datagram(manager, datagram->header.datagram_id);
          }

          manager->rx_state = PKT_STATE_WAITING_SOF;
          manager->bytes_received = 0;
        }
        break;

      default:
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

  /* Fragment datagram into packets */
  FotaPacket packets[FOTA_MAX_PACKETS_PER_DATAGRAM];
  uint16_t num_packets = 0;

  FotaError err = fota_datagram_to_packets(datagram, packets, &num_packets, FOTA_MAX_PACKETS_PER_DATAGRAM);
  if (err != FOTA_ERROR_SUCCESS) {
    return err;
  }

  /* Serialize and send each packet using manager->tx_packet_buffer */
  for (uint16_t i = 0; i < num_packets; ++i) {
    uint32_t written = 0;
    err = fota_packet_serialize(&packets[i], manager->tx_packet_buffer, sizeof(manager->tx_packet_buffer), &written);
    if (err != FOTA_ERROR_SUCCESS) {
      return err;
    }

    err = send_func((int8_t *)manager->tx_packet_buffer, written);
    if (err != FOTA_ERROR_SUCCESS) {
      return err;
    }
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError packet_manager_create_datagram(PacketManager *manager, FotaDatagramType type, uint8_t *data, uint32_t length, FotaDatagram **datagram) {
  if (manager == NULL || datagram == NULL || (data == NULL && length > 0)) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  for (uint8_t i = 0; i < FOTA_MAX_ACTIVE_DATAGRAMS; ++i) {
    if (!manager->datagram_active[i]) {
      *datagram = &manager->active_datagrams[i];
      FotaError err = fota_datagram_init(*datagram, type, s_next_datagram_id++, data, length);
      if (err != FOTA_ERROR_SUCCESS) {
        *datagram = NULL;
        return err;
      }
      manager->datagram_active[i] = true;
      return FOTA_ERROR_SUCCESS;
    }
  }

  return FOTA_ERROR_NO_MEMORY;
}

FotaError packet_manager_get_datagram(PacketManager *manager, uint32_t datagram_id, FotaDatagram **datagram) {
  if (manager == NULL || datagram == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  for (uint8_t i = 0; i < FOTA_MAX_ACTIVE_DATAGRAMS; ++i) {
    if (manager->datagram_active[i] && manager->active_datagrams[i].header.datagram_id == datagram_id) {
      *datagram = &manager->active_datagrams[i];
      return FOTA_ERROR_SUCCESS;
    }
  }

  *datagram = NULL;
  return FOTA_ERROR_NO_DATAGRAM_FOUND;
}

FotaError packet_manager_free_datagram(PacketManager *manager, uint32_t datagram_id) {
  if (manager == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  for (uint8_t i = 0; i < FOTA_MAX_ACTIVE_DATAGRAMS; ++i) {
    if (manager->datagram_active[i] && manager->active_datagrams[i].header.datagram_id == datagram_id) {
      manager->datagram_active[i] = false;
      memset(&manager->active_datagrams[i], 0, sizeof(FotaDatagram));
      return FOTA_ERROR_SUCCESS;
    }
  }

  return FOTA_ERROR_NO_DATAGRAM_FOUND;
}
