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

StatusCode crc_init(CRCLength crc_length);

uint32_t crc_calculate_32(const uint32_t *data, size_t length);
uint32_t crc_calculate_16(const uint16_t *data, size_t length); 
uint32_t crc_calculate_8(const uint8_t *data, size_t length);

void crc_reset(void);

/** @} */
