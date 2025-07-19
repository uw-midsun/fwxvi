#pragma once

/************************************************************************************************
 * @file    fota_encryption.h
 *
 * @brief   Fota Encryption Header File
 *
 * @date    2025-04-29
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_datagram.h"
#include "fota_error.h"
#include "fota_packet.h"

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

/**
 * @brief   Initialize the encryption unit
 * @return  FOTA_SUCCESS if initialized successfully
 *          FOTA_ERROR_INTERNAL_ERROR if initialization fails
 */
FotaError fota_encryption_init();

/**
 * @brief   Calculate the CRC32 for a given buffer of 4-byte aligned data
 * @param   data_start Pointer to the start of the data
 * @param   length_words Size of the data in words
 */
uint32_t fota_calculate_crc32(uint8_t *data, uint32_t length_words);

/**
 * @brief   Verify a FOTA packets encryption
 * @param   packet Pointer to the encrypted packet
 * @return  FOTA_SUCCESS if encryption is decoded successfully and the datagram is valid
 */
FotaError fota_verify_packet_encryption(FotaPacket *packet);

/**
 * @brief   Verify a FOTA datagrams encryption
 * @param   datagram Pointer to the encrypted datagram
 * @return  FOTA_SUCCESS if encryption is decoded successfully and the datagram is valid
 */
FotaError fota_verify_datagram_encryption(FotaDatagram *datagram);

/** @} */
