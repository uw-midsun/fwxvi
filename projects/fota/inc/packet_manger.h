#pragma once

#include "packet.h"

#define PACKET_NUM 8

typedef struct {
    Packet rx_packets[PACKET_NUM];
    uint32_t crc32_expected;
    uint32_t crc32_calculated;
    Packet tx_packet;
} PacketManager;

FotaError crc_check(PacketManager* packet_manager);

FotaError transmit_packet(PacketManager* packet_manager);
