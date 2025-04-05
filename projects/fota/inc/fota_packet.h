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

#define PAYLOAD_SIZE 256


typedef union{
  uint8_t raw_payload[PAYLOAD_SIZE];
  struct{
    uint32_t crc;
    uint8_t data[PAYLOAD_SIZE-sizeof(uint32_t)];
  }data_pakcet;
  }PacketPayload;
typedef enum { HEADER_PACKET, DATA_PACKET, ERROR_PACKET } PacketType;
typedef struct {
  uint8_t sof;
  PacketType packet_type;
  uint8_t sequence_num;
  uint16_t payload_length;
  uint8_t payload[PAYLOAD_SIZE];
  uint8_t eof;
} Packet;

void packet_init(Packet* Packet, uint);
FotaError encode_packet(Packet *packet, uint8_t *payload);

FotaError decode_packet(CircularBuffer *buffer);

/** @} */
