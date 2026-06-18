#pragma once

/************************************************************************************************
 * @file   datagram.h
 *
 * @brief  Datagram header file for telemetry
 *
 * @date   2025-01-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "can.h"

/* Intra-component Headers */

/**
 * @defgroup telemetry
 * @brief    telemetry Firmware
 * @{
 */

/** @brief  Datagram buffer size */
#define DATAGRAM_BUFFER_SIZE 64U
/** @brief  Start of frame indicator */
#define DATAGRAM_START_FRAME 0xAAU
/** @brief  End of frame indicator */
#define DATAGRAM_END_FRAME 0xBBU

/**
 * @brief   Datagram metadata size
 * @details 1 byte for start frame
 *          2 bytes for ID
 *          1 byte for DLC
 *          1 byte for end frame
 */
#define DATAGRAM_METADATA_SIZE 5U

#pragma pack(push, 1)
/**
 * @brief   Datagram storage class
 */
typedef struct {
  uint8_t start_frame; /**< Should always be initialized to 0xAA */
  uint16_t id;         /**< CAN message ID */
  uint8_t dlc;         /**< Data Length Code: number of bytes in the message payload */
  uint8_t data[9];     /**< Holds the payload; maximum size is 8 bytes + 1 for end_frame */
} Datagram;
#pragma pack(pop)

/**
 * @brief   Decode CAN message and update a datagram
 * @param   datagram Pointer to the datagram to be updated
 * @param   msg Pointer to the new CAN data
 */
StatusCode decode_can_message(Datagram* datagram, CanMessage* msg);

/**
 * @brief   Encode id/dlc/data directly into a datagram (no CanMessage wrapper).
 * @param   datagram Pointer to the datagram to populate
 * @param   id       Raw CAN ID
 * @param   dlc      Payload length in bytes
 * @param   data     Pointer to payload
 */
StatusCode encode_datagram(Datagram* datagram, uint32_t id, uint8_t dlc, const uint8_t* data);

/**
 * @brief   Log the decoded datagram for debug purposes
 * @param   datagram Pointer to the datagram to be debugged
 */
void log_decoded_message(Datagram* datagram);

/** @} */
