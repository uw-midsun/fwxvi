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
#define FOTA_MAX_DATAGRAM_SIZE (16U * FOTA_PACKET_PAYLOAD_SIZE)

/** @brief Maximum number of packets that can make up a datagram (8 data + 1 Header) */
#define FOTA_MAX_PACKETS_PER_DATAGRAM ((FOTA_MAX_DATAGRAM_SIZE / FOTA_PACKET_PAYLOAD_SIZE) + 1U)

/** @brief FOTA Datagram header payload size of 10-bytes */
#define FOTA_DATAGRAM_HEADER_PAYLOAD_SIZE 10U

/**
 * @brief Datagram types corresponding to different FOTA operations
 */
typedef enum FotaDatagramType {
  FOTA_DATAGRAM_TYPE_FIRMWARE_METADATA, /**< Firmware metadata information */
  FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK,    /**< Chunk of firmware data */
  FOTA_DATAGRAM_TYPE_JUMP_TO_APP,       /**< Request to jump to application */
  FOTA_DATAGRAM_TYPE_ACKNOWLEDGEMENT,   /**< Acknowledgement information */
  FOTA_DATAGRAM_TYPE_ERROR
} FotaDatagramType;

/**
 * Datagram Header Payload Format (20 bytes total):
 *
 *  +-------------------+----------------+---------------------------------------------------------------+
 *  | Field             | Size (bytes)   | Description                                                  |
 *  +-------------------+----------------+---------------------------------------------------------------+
 *  | target_node_id    | 1              | Target node ID (0 = RF Board, 1,2... = CAN Bootloader board)|
 *  | type              | 1              | Type of datagram (enum FotaDatagramType)                     |
 *  | num_packets       | 2              | Number of data packets in this datagram (excluding header)  |
 *  | datagram_id       | 4              | Unique identifier for this datagram                          |
 *  | total_length      | 4              | Total length of the datagram data                             |
 *  | datagram_crc32    | 4              | CRC32 of the entire datagram data                             |
 *  | packet_crc32      | 4              | CRC32 of this header packet payload                            |
 *  +-------------------+----------------+---------------------------------------------------------------+
 *
 *
 * All fields are encoded in little-endian format.


/**
 * @brief Structure representing the datagram header (In the payload)
 */
typedef struct {
  uint8_t target_node_id;  /**< Target node ID */
  FotaDatagramType type;   /**< Type of datagram */
  uint16_t num_packets;    /**< Number of packets in this datagram excluding the header packet */
  uint32_t datagram_id;    /**< Unique ID for this datagram */
  uint32_t total_length;   /**< Total length of datagram content */
  uint32_t datagram_crc32; /**< CRC of the entire datagram for verification */
  uint32_t packet_crc32;   /**< CRC of the header packet */
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
 * @param    num_packets Pointer to store number of packets generated (Including the header packet)
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
