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

/** @brief  Error message length in bytes */
#define FOTA_ERROR_MSG_LENGTH 256U

/** @brief  Firmware ID length in bytes */
#define FOTA_FIRMWARE_ID_LENGTH 256U

typedef struct {
  uint32_t binary_size;                      /**< Binary size */
  uint32_t expected_binary_crc32;            /**< Expected binary CRC32 */
  uint8_t version_major;                     /**< Version number (major) */
  uint8_t version_minor;                     /**< Version number (minor) */
  char firmware_id[FOTA_FIRMWARE_ID_LENGTH]; /**< Firmware name/ID */
} FotaDatagramPayload_FirmwareMetadata;

typedef struct {
  uint8_t data[FOTA_MAX_DATAGRAM_SIZE]; /**< 2048 Byte data buffer */
} FotaDatagramPayload_FirmwareChunk;

typedef struct {
  uint8_t jump_location;   /**< Jump location (see #FotaJumpRequest) */
  uint32_t magic_number;   /**< Midnight sun magic number to prevent unauthorized jumps */
  uint8_t validation_flag; /**< 1 = OK to jump, 0 = stay in bootloader */
} FotaDatagramPayload_JumpToApp;

typedef struct {
  uint8_t ack_status;                    /**< 0 == ACK, 1 == NACK */
  uint8_t response_to_type;              /**< Response to type (see #FotaDatagramType) */
  uint32_t error_code;                   /**< Error code (see #FotaError) */
  char error_msg[FOTA_ERROR_MSG_LENGTH]; /**< Error message (optional) */
} FotaDatagramPayload_Acknowledgement;

/** @} */
