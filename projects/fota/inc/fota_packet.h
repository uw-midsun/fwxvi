#pragma once

/************************************************************************************************
 * @file   fota_packet.h
 *
 * @brief  Packet implementation for firmware over the air (FOTA) updates
 *
 * @date   2025-03-16
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_error.h"

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

/**
 * FotaPacket Serialized Format:
 *
 *  +------------+----------------+--------------------------+
 *  | Field      | Size (bytes)   | Description              |
 *  +------------+----------------+--------------------------+
 *  | sof        | 1              | Start of Frame (0xAA)    |
 *  | packet_type| 1              | Packet Type Enum         |
 *  | datagram_id| 4              | Datagram identifier      |
 *  | sequence_num| 1             | Packet index in datagram |
 *  | payload_len| 2              | Actual data in payload   |
 *  | payload    | 128            | Fixed-size payload       |
 *  | crc32      | 4              | CRC32 of full payload    |
 *  | eof        | 1              | End of Frame (0xBB)      |
 *  +------------+----------------+--------------------------+
 *
 * Total Serialized Size: 142 bytes
 */

#define FOTA_PACKET_SOF 0xAAU
#define FOTA_PACKET_EOF 0xBBU
#define FOTA_PACKET_PAYLOAD_SIZE 128U
#define FOTA_PACKET_HEADER_SIZE (1U + 1U + 4U + 1U + 2U)
#define FOTA_PACKET_SERIALIZED_SIZE (FOTA_PACKET_HEADER_SIZE + FOTA_PACKET_PAYLOAD_SIZE + 4U + 1U)
#define FOTA_PACKET_MINIMUM_SIZE FOTA_PACKET_SERIALIZED_SIZE

/**
 * @brief FOTA Packet Type
 */
typedef enum {
  FOTA_PACKET_TYPE_DATA = 0, /**< Data packet */
  FOTA_PACKET_TYPE_HEADER,   /**< Header packet */
  FOTA_PACKET_TYPE_ACK,      /**< Acknowledgment packet */
  FOTA_PACKET_TYPE_NACK,     /**< Not Acknowledged packet */
  FOTA_PACKET_TYPE_ERROR     /**< Error packet */
} FotaPacketType;

/**
 * @brief FOTA Packet structure
 *
 * Note: Only `payload_length` bytes in `payload` are meaningful.
 * Remaining bytes (padding) must be zeroed before CRC.
 */
typedef struct {
  uint8_t sof;                               /**< Start of Frame (0xAA) */
  FotaPacketType packet_type;                /**< Packet Type */
  uint32_t datagram_id;                      /**< Datagram ID */
  uint8_t sequence_num;                      /**< Sequence in datagram */
  uint16_t payload_length;                   /**< Length of meaningful data in payload */
  uint8_t payload[FOTA_PACKET_PAYLOAD_SIZE]; /**< Fixed-size payload (zero-padded) */
  uint32_t crc32;                            /**< CRC32 of payload */
  uint8_t eof;                               /**< End of Frame (0xBB) */
} FotaPacket;

/**
 * @brief Initialize a FOTA packet
 * @param packet Pointer to packet
 * @param type Packet type
 * @param sequence Sequence number
 * @param length Length of actual data (≤ FOTA_PACKET_PAYLOAD_SIZE, multiple of 4)
 * @return FOTA_ERROR_SUCCESS or error
 */
FotaError fota_packet_init(FotaPacket *packet, FotaPacketType type, uint8_t sequence, uint16_t length);

/**
 * @brief Compute and set CRC32 of payload
 * @param packet Pointer to packet
 * @return FOTA_ERROR_SUCCESS or error
 */
FotaError fota_packet_set_crc(FotaPacket *packet);

/**
 * @brief Serialize FOTA packet into buffer
 * @param packet Packet to serialize
 * @param buffer Output buffer
 * @param buf_size Size of output buffer
 * @param bytes_written Number of bytes written
 * @return FOTA_ERROR_SUCCESS or error
 */
FotaError fota_packet_serialize(FotaPacket *packet, uint8_t *buffer, uint32_t buf_size, uint32_t *bytes_written);

/**
 * @brief Deserialize buffer into FOTA packet
 * @param packet Output FotaPacket struct
 * @param buffer Input buffer
 * @param buf_size Size of input buffer
 * @return FOTA_ERROR_SUCCESS or error
 */
FotaError fota_packet_deserialize(FotaPacket *packet, uint8_t *buffer, uint32_t buf_size);

/** @} */
