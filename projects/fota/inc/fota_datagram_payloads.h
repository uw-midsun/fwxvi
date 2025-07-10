#pragma once

/************************************************************************************************
 * @file    fota_datagram.h
 *
 * @brief   Fota Datagram Header File
 *
 * @date    2025-04-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_datagram.h"

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

/**
 * @brief Datagram types corresponding to different FOTA operations
 */
typedef enum FotaDatagramType {
  FOTA_DATAGRAM_TYPE_FIRMWARE_METADATA, /**< Firmware metadata information */
  FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK,    /**< Chunk of firmware data */
  FOTA_DATAGRAM_TYPE_JUMP_TO_APP,       /**< Request to jump to application */
  FOTA_DATAGRAM_TYPE_ACKNOWLEDGEMENT,   /**< Acknowledgement information */
  FOTA_DATAGRAM_TYPE_ERROR
} FotaDatagramType;

typedef struct {
    uint32_t binary_size;
    uint32_t expected_binary_crc32;
} FotaDatagramPayload_FirmwareMetadata;

typedef struct {
    uint8_t data[FOTA_MAX_DATAGRAM_SIZE];
} FotaDatagramPayload_FirmwareChunk;

typedef struct {

} FotaDatagramPayload_JumpToApp;

typedef struct {
    
} FotaDatagramPayload_Acknowledgement;

/** @} */
