#pragma once

/************************************************************************************************
 * @file    fota_datagram.h
 *
 * @brief   Fota Datagram Header File
 *
 * @date    2025-04-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_packet.h"

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

/** @brief Maximum supported datagram size (2KB) */
#define FOTA_MAX_DATAGRAM_SIZE (8U * 256U)

/** @brief Maximum number of packets that can make up a datagram (8 data + 1 Header) */
#define FOTA_MAX_PACKETS_PER_DATAGRAM ((FOTA_MAX_DATAGRAM_SIZE / FOTA_PACKET_PAYLOAD_SIZE) + 1U)

/**
 * @brief Datagram types corresponding to different FOTA operations
 */
typedef enum {
  FOTA_DATAGRAM_TYPE_FIRMWARE_METADATA, /**< Firmware metadata information */
  FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK,    /**< Chunk of firmware data */
  FOTA_DATAGRAM_TYPE_UPDATE_REQUEST,    /**< Request to perform update */
  FOTA_DATAGRAM_TYPE_UPDATE_RESPONSE,   /**< Response to update request */
  FOTA_DATAGRAM_TYPE_VERIFICATION       /**< Verification information */
} FotaDatagramType;

/**
 * @brief Structure representing the datagram header (In the payload)
 */
typedef struct {
  FotaDatagramType type; /**< Type of datagram */
  uint32_t datagram_id;  /**< Unique ID for this datagram */
  uint32_t total_length; /**< Total length of datagram content */
  uint16_t num_packets;  /**< Number of packets in this datagram */
  uint32_t crc32;        /**< CRC of the entire datagram for verification */
} FotaDatagramHeaderPacketPayload;

/**
 * @brief   Structure representing datagram
 */
typedef struct {
  FotaDatagramHeaderPacketPayload header;              /**< Datagram header */
  uint8_t data[FOTA_MAX_DATAGRAM_SIZE];                /**< Datagram payload */
  bool packet_received[FOTA_MAX_PACKETS_PER_DATAGRAM]; /**< Bitmap for tracking which packets have been received */
  uint16_t packets_received;                           /**< Count of received packets */
  bool is_complete;                                    /**< Whether all packets have been received */
} FotaDatagram;

/**
 * @brief   Initialize a datagram structure
 * @param   datagram Pointer to datagram to initialize
 * @param   type Type of datagram
 * @param   datagram_id Unique ID for this datagram
 * @param   data Data to include in datagram (can be NULL if building incrementally)
 * @param   length Length of data
 * @return  FotaError  Error code
 */
FotaError fota_datagram_init(FotaDatagram *datagram, FotaDatagramType type, uint32_t datagram_id, uint8_t *data, uint32_t length);

/**
 * @brief    Fragment a datagram into packets for transmission
 * @param    datagram    Datagram to fragment
 * @param    packets     Array to store generated packets
 * @param    num_packets Pointer to store number of packets generated
 * @param    max_packets Maximum number of packets that can be stored in the array
 * @return   FotaError  Error code
 */
FotaError fota_datagram_to_packets(FotaDatagram *datagram, FotaPacket *packets, uint16_t *num_packets, uint16_t max_packets);

/**
 * @brief    Process a received packet and incorporate it into a datagram
 * @param    datagram Datagram being reconstructed
 * @param    packet Received packet
 * @return   FotaError  Error code
 */
FotaError fota_datagram_process_data_packet(FotaDatagram *datagram, FotaPacket *packet);

/**
 * @brief    Process a received header packet and incorporate it into a datagram
 * @param    datagram Datagram being reconstructed
 * @param    packet Received header packet
 * @return   FotaError  Error code
 */
FotaError fota_datagram_process_header_packet(FotaDatagram *datagram, FotaPacket *packet);

/**
 * @brief    Check if a datagram has been completely received
 * @param    datagram Datagram to check
 * @return   TRUE if datagram is complete
 *           FALSE otherwise
 */
bool fota_datagram_is_complete(FotaDatagram *datagram);

/**
 * @brief   Verify the integrity of a complete datagram
 * @param   datagram Datagram to verify
 * @return  FotaError  Error code
 */
FotaError fota_datagram_verify(FotaDatagram *datagram);

/** @} */
