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
#include "fota_error.h"

FotaError can_bootloader_init(void) {
    return FOTA_ERROR_SUCCESS;
}

FotaError can_bootloader_transmit(uint32_t id, bool extended, uint8_t *data, uint8_t len) {
    return FOTA_ERROR_SUCCESS;
}

FotaError can_bootloader_receive(uint32_t id, uint8_t *data, uint8_t *len) {
    return FOTA_ERROR_SUCCESS;
}

FotaError can_bootloader_poll(void) {
    return FOTA_ERROR_SUCCESS;
}

FotaError can_bootloader_chunkify(void) {
    return FOTA_ERROR_SUCCESS;
}
