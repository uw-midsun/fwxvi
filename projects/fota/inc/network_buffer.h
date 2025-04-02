#pragma once

/************************************************************************************************
 * @file   network_buffer.h
 *
 * @brief  Circular Buffer class implementation for firmware over the air (FOTA) updates
 *
 * @date   2025-03-16
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/


/* Standard library Headers */
#include <stdint.h>
#include <stdbool.h>

/* Inter-component Headers */

/* Intra-component Headers */

/* Constants */
//subject to change, but these were numbers I got from confluence
#define MAX_PACKETS 8 
#define PACKET_BYTES 256

#define MAX_BYTES 1024

/**
 * @brief struct containing details of circular buffer
 * 
 * @param data pointer to internally held data
 * @param size size of circular buffer
 * @param insert current insert index
 * @param head current read index
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
    SUCCESS = 0,
    ALREADY_INITALISED,
    NOT_INITALIZED,
    BUFFER_ERROR
} returnStatus;

/**
 * @brief data packet
 * @param data pointer to data bytes
 */
typedef struct packet {
    uint8_t internal[PACKET_BYTES];
} packet;

/**
 * @brief Initalizes circular buffer for use, by initalizing assocated structs
 * 
 * @return returnStatus
*/
returnStatus network_buffer_init();

/**
 * @brief Insert data packet 
 * 
 * @param data pointer to data packet to insert
 * @return returnStatus 
 */
returnStatus network_buffer_insert(packet *data);

/**
 * @brief 
 * 
 * @param buf reads packet from circular buffer into a provided location
 * @return returnStatus 
 */
returnStatus network_buffer_read(packet *loc);
