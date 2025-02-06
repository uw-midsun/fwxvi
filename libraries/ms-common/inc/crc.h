#pragma once

/************************************************************************************************
 * @file   crc.h
 *
 * @brief  CRC Library Header file
 *
 * @date   2025-02-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "status.h"

/**
 * @defgroup CRC
 * @brief    CRC library
 * @{
 */

/** @brief  CRC Length */
typedef enum {
  CRC_LENGTH_7 = 0,
  CRC_LENGTH_8,         /**< 100kHz */
  CRC_LENGTH_16,        /**< 400 kHz */
  CRC_LENGTH_32,        /**< Number of I2C Speeds */
  NUM_CRC_LENGTHS
} CRCLength;


/**
 * @brief Initilize the CRC api with default values
 * @param crc_length Number of bits in the CRC
 * @return STATUS_CODE_INVALID_ARGS if invalid crc_length is entered.
 * @return STATUS_CODE_INTERNAL_ERROR if CRC initialization failed, possibly due to invalid parameters,
           hardware issues, or improper clock configuration.
 * @return STATUS_CODE_OK is returned if CRC api was initilized successfully 
 */
StatusCode crc_init(CRCLength crc_length);

/**
 * @brief  Calculate the CRC-32 checksum for a buffer of 32-bit values
 * @param data Pointer to the buffer of 32-bit words
 * @param length Number of 32-bit words in the buffer
 * @return Computed CRC-32 checksum
 */
uint32_t crc_calculate_32(const uint32_t *data, size_t length);

/**
 * @brief Calculate the CRC-32 checksum for a buffer of 16-bit values
 * @param data Pointer to the buffer of 16-bit values
 * @param length Number of 16-bit values in the buffer
 * @return Computed CRC-32 checksum
 */
uint32_t crc_calculate_16(const uint16_t *data, size_t length);

/**
 * @brief Calculate the CRC-32 checksum for a buffer of 8-bit values
 * @param   data Pointer to the buffer of 8-bit values
 * @param   length Number of 8-bit values in the buffer
 * @return  Computed CRC-32 checksum
 */
uint32_t crc_calculate_8(const uint8_t *data, size_t length);

/** 
 * @brief   Reset the CRC hardware peripheral
 * @details This function forces a reset of the CRC peripheral and then releases the reset,
 *          ensuring that all CRC registers are cleared and reset to their default state.
*/
void crc_reset(void);

/** @} */
