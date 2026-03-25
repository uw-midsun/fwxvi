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
  CRC_LENGTH_8,   /**< CRC Length of 8 bits */
  CRC_LENGTH_16,  /**< CRC Length of 16 bits */
  CRC_LENGTH_32,  /**< CRC Length of 32 bits */
  NUM_CRC_LENGTHS /**< Number of CRC Length */
} CRCLength;

/**
 * @brief   Initilize the CRC API with default values
 * @param   crc_length Number of bits in the CRC
 * @return  STATUS_CODE_INVALID_ARGS if invalid crc_length is entered.
 * @return  STATUS_CODE_INTERNAL_ERROR if CRC initialization failed, possibly due to invalid parameters,
            hardware issues, or improper clock configuration.
 * @return  STATUS_CODE_OK is returned if CRC API was initilized successfully
 */
StatusCode crc_init(CRCLength crc_length);

/**
 * @brief   Calculate the CRC-32/16/8 checksum for a buffer of 32-bit values
 * @details This expects the user to determine the number of CRC-length sized words in the data
 *          array. For example, if CRC is initialized with length 16, the user must
 *          divide the total size of the array by 2, for the number of 16-bit words
 * @param   data Pointer to the buffer of 32-bit words
 * @param   length Number of CRC-length words in the buffer
 * @return  Computed CRC-32 checksum
 */
uint32_t crc_calculate(const uint32_t *data, size_t length);

/**
 * @brief   Reset the CRC hardware peripheral
 * @details This function forces a reset of the CRC peripheral and then releases the reset,
 *          ensuring that all CRC registers are cleared and reset to their default state.
 */
void crc_reset(void);

/** @} */
