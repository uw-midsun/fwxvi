#pragma once

/************************************************************************************************
 * @file   packet_manager.h
 *
 * @brief  Packet manager class for firmware over the air (FOTA) updates
 *
 * @date   2025-03-16
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_datagram.h"
#include "fota_error.h"
#include "fota_packet.h"
#include "network_buffer.h"

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

/**
 * @brief   State machine states for packet receiving
 */
typedef enum {
  PKT_STATE_WAITING_SOF,     /**< Waiting for start of frame */
  PKT_STATE_READING_HEADER,  /**< Reading packet header */
  PKT_STATE_READING_PAYLOAD, /**< Reading packet payload */
  PKT_STATE_READING_CRC,     /**< Reading CRC */
  PKT_STATE_READING_EOF      /**< Reading end of frame */
} PacketReceiverState;

/* Maximum number of datagrams that can be processed simultaneously */
#define FOTA_MAX_ACTIVE_DATAGRAMS 2U

/**
 * @brief Packet manager context structure
 */
typedef struct {
  /* Network buffer for raw data */
  NetworkBuffer network_buffer;

  /* Packet reception */
  PacketReceiverState rx_state;
  uint16_t bytes_received;
  uint8_t packet_buffer[sizeof(FotaPacket)];
  FotaPacket current_packet;

  /* Datagram handling */
  FotaDatagram active_datagrams[FOTA_MAX_ACTIVE_DATAGRAMS];
  bool datagram_active[FOTA_MAX_ACTIVE_DATAGRAMS];

  /* Callback for completed datagrams */
  void (*datagram_complete_callback)(FotaDatagram *datagram);
} PacketManager;

/**
 * @brief   Initialize the packet manager
 * @param   manager Pointer to packet manager structure
 * @param   network_buffer Pointer to network buffer to use
 * @param   callback Callback function for completed datagrams (can be NULL)
 * @return  Error code
 */
FotaError packet_manager_init(PacketManager *manager, NetworkBuffer *network_buffer, void (*callback)(FotaDatagram *datagram));

/**
 * @brief   Process received data from the network buffer
 * @details This function shall be called periodically to process any data in the network buffer
 *          It will extract bytes from the buffer, assemble them into packets, and then
 *          assemble packets into datagrams.
 * @param   manager Pointer to packet manager structure
 * @return  Error code
 */
FotaError packet_manager_process(PacketManager *manager);

/**
 * @brief   Send a datagram by fragmenting it into packets
 * @param   manager Pointer to packet manager structure
 * @param   datagram Datagram to send
 * @param   send_func Function to call to send each packet
 * @return  Error code
 */
FotaError packet_manager_send_datagram(PacketManager *manager, FotaDatagram *datagram, FotaError (*send_func)(int8_t *data, uint32_t length));

/**
 * @brief   Create a new datagram for sending
 * @param   manager Pointer to packet manager structure
 * @param   type Type of datagram to create
 * @param   data Data to include in datagram
 * @param   length Length of data
 * @param   datagram Pointer to store the created datagram
 * @return  Error code
 */
FotaError packet_manager_create_datagram(PacketManager *manager, FotaDatagramType type, uint8_t *data, uint32_t length, FotaDatagram *datagram);

/**
 * @brief   Get a pointer to a completed datagram
 * @param   manager Pointer to packet manager structure
 * @param   datagram_id ID of the datagram to retrieve
 * @param   datagram Pointer to store pointer to datagram
 * @return  Error code
 */
FotaError packet_manager_get_datagram(PacketManager *manager, uint32_t datagram_id, FotaDatagram **datagram);

/**
 * @brief   Free a datagram slot
 * @param   manager Pointer to packet manager structure
 * @param   datagram_id ID of the datagram to free
 * @return  Error code
 */
FotaError packet_manager_free_datagram(PacketManager *manager, uint32_t datagram_id);

/** @} */
