/************************************************************************************************
 * @file    can_bootloader.c
 *
 * @brief   CAN Bootloader API
 *
 * @date    2025-06-04
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "can_bootloader.h"
#include "bootloader_can.h"
#include "fota_error.h"

static uint8_t fota_chunk_buffer[FOTA_CHUNK_SIZE];
static size_t chunk_offset = 0;
static Can_StateMachine current_state = CAN_STATE_READY;

FotaError can_bootloader_init(void) {
    Boot_CanSettings settings = {
        .bitrate = BOOT_CAN_BITRATE_500KBPS,
        .loopback = false,
        .silent = false
    };

    if (boot_can_init(&settings) != BOOTLOADER_ERROR_NONE) {
        return FOTA_ERROR_CAN_INIT;
    }

    current_state = CAN_STATE_WAIT_FOR_START;
    chunk_offset = 0;
    memset(fota_chunk_buffer, 0, sizeof(fota_chunk_buffer));
    return FOTA_ERROR_SUCCESS;
}

FotaError can_bootloader_transmit(uint32_t id, bool extended, uint8_t *data, uint8_t len) {
    if (data == NULL || len > 8) {
        return FOTA_ERROR_INVALID_ARGS;
    }

    if (boot_can_transmit(id, extended, data, len) != BOOTLOADER_ERROR_NONE) {
        return FOTA_ERROR_CAN_TRANSMIT;
    }

    return FOTA_ERROR_SUCCESS;
}

FotaError can_bootloader_receive(uint32_t id, uint8_t *data, uint8_t *len) {
    if (data == NULL || len == NULL) {
        return FOTA_ERROR_INVALID_ARGS;
    }

    Boot_CanMessage msg;
    if (boot_can_receive(&msg) != BOOTLOADER_ERROR_NONE) {
        return FOTA_ERROR_CAN_RECEIVE;
    }

    if (msg.id != id) {
        return FOTA_ERROR_INVALID_PACKET;
    }

    memcpy(data, msg.data_u8, msg.dlc);
    *len = msg.dlc;
    return FOTA_ERROR_SUCCESS;
}

FotaError can_bootloader_poll(void) {
    Boot_CanMessage msg;
    if (boot_can_receive(&msg) != BOOTLOADER_ERROR_NONE) {
        return FOTA_ERROR_CAN_RECEIVE;
    }

    switch (current_state) {
        case CAN_STATE_WAIT_FOR_START:
            if (msg.data_u8[0] == FOTA_CMD_START) {
                current_state = CAN_STATE_RECIEVE_CHUNK;
            }
            break;

        case CAN_STATE_RECIEVE_CHUNK:
            if (msg.data_u8[0] == FOTA_CMD_CHUNK) {
                return can_bootloader_chunkify();
            }
            break;

        default:
            break;
    }

    return FOTA_ERROR_SUCCESS;
}

FotaError can_bootloader_chunkify(void) {
    Boot_CanMessage msg;
    if (boot_can_receive(&msg) != BOOTLOADER_ERROR_NONE) {
        return FOTA_ERROR_CAN_RECEIVE;
    }

    if (msg.data_u8[0] != FOTA_CMD_CHUNK) {
        return FOTA_ERROR_INVALID_CMD;
    }

    size_t payload_len = msg.dlc - 1;
    if (chunk_offset + payload_len > FOTA_CHUNK_SIZE) {
        return FOTA_ERROR_OVERFLOW;
    }

    memcpy(&fota_chunk_buffer[chunk_offset], &msg.data_u8[1], payload_len);
    chunk_offset += payload_len;

    if (chunk_offset >= FOTA_CHUNK_SIZE) {
        chunk_offset = 0;
        current_state = CAN_STATE_VALIDATE_CHUNK;
    }

    return FOTA_ERROR_SUCCESS;
}
