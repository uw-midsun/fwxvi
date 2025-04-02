#pragma once

#include <stdint.h>
#include "error_fota.h"

#define PAYLOAD_SIZE 256

typedef struct {
    uint8_t sof;
    PacketType packet_type; 
    uint8_t sequence_num;
    uint16_t payload_length;
    uint8_t payload[PAYLOAD_SIZE];
    uint8_t eof;
} Packet;

typedef enum PacketType {
    HEADER_PACKET,
    DATA_PACKET,
    ERROR_PACKET
};

FotaError encode_packet(Packet* packet, uint8_t* payload);

FotaError decode_packet(Packet* packet, uint8_t* payload);


