#pragma once

/************************************************************************************************
 * @file    tcixtma1_crc16.h
 *
 * @brief   Header file to implement CRC-16/CCITT-FALSE for the TCIXTMA1
 *
 * @date    2026-06-07
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/
/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup TCIXTMA1
 * @brief    TCIXTMA1 library
 * @{
 */

/**
 * @brief Initiliaze the CRC16 table
 * @details Used for fast CRC16 lookups
 */
void tcixtma1_crc16_init_table(void);

/**
 * @brief Calculates CRC16 value for given data
 *
 * @param data the data to calculate the CRC for
 * @param len length of the data
 * @return uint16_t return CRC value
 */
uint16_t tcixtma1_crc16_calculate(uint8_t *data, size_t len);

/** @} */
