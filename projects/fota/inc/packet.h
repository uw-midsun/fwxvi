#pragma once

#include <stdint.h>
#include "error_fota.h"

#define PAYLOAD_SIZE 256

typedef enum {
    HEADER_PACKET,
    DATA_PACKET,
    ERROR_PACKET
} PacketType;
typedef struct {
    uint8_t sof;
    PacketType packet_type; 
    uint8_t sequence_num;
    uint16_t payload_length;
    uint8_t payload[PAYLOAD_SIZE];
    uint8_t eof;
} Packet;

FotaError encode_packet(Packet* packet, uint8_t* payload);

FotaError decode_packet(Packet* packet, uint8_t* payload);
