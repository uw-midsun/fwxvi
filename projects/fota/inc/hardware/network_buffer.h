#pragma once
#include <stdint.h>
#include <stdbool.h>

#define MAX_PACKETS 8 //TODO: Figure out what this size is
#define PACKET_BYTES 256



/**
 * @brief struct containing details of circular buffer
 * 
 */
typedef struct NetworkBuffer {
    packet *data;
    uint32_t size;
    uint32_t insert;
    uint32_t head;
} NetworkBuffer;

/**
 * @brief List of possbile return statuses for circular buffer
 * 
 */
typedef enum returnStatus {
    SUCCESS,
    ALREADY_INITALISED,
    FAILED
} returnStatus;


/**
 * @brief data packet which contains 256 bytes
 * 
 */
typedef struct packet {
    uint8_t data[PACKET_BYTES];
} packet;

static packet data[MAX_PACKETS];
static NetworkBuffer circular_buffer = {data, MAX_PACKETS, 0, 0 };

static bool init = false;


/**
 * @brief Initalizes circular buffer for use, by initalizing assocated structs
 * 
 * @return returnStatus: status of initalization
 */
returnStatus initCircularBuffer();

/**
 * @brief Insert data packet 
 * 
 * @param data pointer to data packet to insert
 * @return returnStatus 
 */
returnStatus insertNetworkBuffer(packet *data);

/**
 * @brief 
 * 
 * @param buf reads packet from circular buffer into a provided buffer
 * @return returnStatus 
 */
returnStatus readNetworkBuffer(packet *buf);
