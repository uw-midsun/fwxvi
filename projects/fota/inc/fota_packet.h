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
 *  +------------+----------------+------------------+----------------+-------------------+
 *  | Field      | Size (bytes)   | Description      | Example Value  | Notes             |
 *  +------------+----------------+------------------+----------------+-------------------+
 *  | sof        | 1              | Start of Frame   | 0xAA           | Always 0xAA       |
 *  | packet_type| 1              | Packet Type Enum | 0x01           | Type identifier   |
 *  | datagram_id| 4              | Datagram ID      | 0x00000005     | Big/little endian |
 *  | sequence_num| 1             | Packet Index     | 0x02           | 0–7 in a datagram |
 *  | payload_len| 2              | Payload Length   | 0x0040         | In bytes          |
 *  | payload    | N (variable)   | Payload Data     | [...]          | Up to defined max |
 *  | crc32      | 4              | CRC32 Checksum   | 0xDEADBEEF     | On payload        |
 *  | eof        | 1              | End of Frame     | 0xBB           | Always 0xBB       |
 *  +------------+----------------+------------------+----------------+-------------------+
 *
 * Total Size: 14 bytes of header/footer + N bytes payload
 */

/** @brief  FOTA packet payload size */
#define FOTA_PACKET_PAYLOAD_SIZE 256U
/** @brief  FOTA packet start of frame */
#define FOTA_PACKET_SOF 0xAAU
/** @brief FOTA packet end of frame */
#define FOTA_PACKET_EOF 0xBBU
/** @brief FOTA packet minimum size */
#define FOTA_PACKET_MINIMUM_SIZE 14U

/**
 * @brief   FOTA Packet Type
 */
typedef enum {
  FOTA_PACKET_TYPE_DATA,   /**< Data packet */
  FOTA_PACKET_TYPE_HEADER, /**< Header packet */
  FOTA_PACKET_TYPE_ACK,    /**< Acknowledgment packet */
  FOTA_PACKET_TYPE_NACK,   /**< Not Acknowledged packet */
  FOTA_PACKET_TYPE_ERROR   /**< Error packet */
} FotaPacketType;

/**
 * @brief   FOTA Packet structure
 */
typedef struct {
  uint32_t crc32;                            /**< CRC32 on the packet */
  uint32_t datagram_id;                      /**< Datagram ID to match the packet to */
  FotaPacketType packet_type;                /**< Packet type */
  uint16_t payload_length;                   /**< Payload length */
  uint8_t sequence_num;                      /**< Sequence number of the packet within the 8-packet datagram */
  uint8_t sof;                               /**< Packet start of frame (0xAA) */
  uint8_t eof;                               /**< Packet end of frame (0xBB) */
  uint8_t _reserved_padding;                 /**< Reserved padding byte */
  uint8_t payload[FOTA_PACKET_PAYLOAD_SIZE]; /**< Payload data */
} FotaPacket;

/**
 * @brief   Initialize a packet structure
 * @param   packet Pointer to the packet to initialize
 * @param   type Type of packet
 * @param   sequence Sequence number
 * @param   length Payload length
 * @return  FotaError  Error code
 */
FotaError fota_packet_init(FotaPacket *packet, FotaPacketType type, uint8_t sequence, uint16_t length);

/**
 * @brief   Calculate and set the CRC for a packet
 * @param   packet Pointer to packet
 * @return  FotaError  Error code
 */
FotaError fota_packet_set_crc(FotaPacket *packet);

/**
 * @brief   Serialize packet into a buffer for transmission
 * @param   packet Packet to serialize
 * @param   buffer Output buffer
 * @param   buf_size Size of output buffer
 * @param   bytes_written Number of bytes written
 * @return  FotaError  Error code
 */
FotaError fota_packet_serialize(FotaPacket *packet, uint8_t *buffer, uint32_t buf_size, uint32_t *bytes_written);

/**
 * @brief   Deserialize packet from received data
 * @param   packet Output packet structure
 * @param   buffer Input buffer containing serialized packet
 * @param   buf_size Size of input buffer
 * @return  FotaError  Error code
 */
FotaError fota_packet_deserialize(FotaPacket *packet, uint8_t *buffer, uint32_t buf_size);

/** @} */
