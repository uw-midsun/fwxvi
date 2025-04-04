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
#include "fota_error.h"
#include "fota_packet.h"

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

#define PACKET_NUM 8

typedef struct {
  Packet rx_packets[PACKET_NUM];
  uint32_t crc32_expected;
  uint32_t crc32_calculated;
  Packet tx_packet;
} PacketManager;

FotaError packet_manager_init(PacketManager *packet_manager);

FotaError crc_check(PacketManager *packet_manager);

FotaError transmit_packet(PacketManager *packet_manager);

/** @} */
