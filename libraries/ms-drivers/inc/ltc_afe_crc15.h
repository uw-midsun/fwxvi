#pragma once

/************************************************************************************************
 * @file   ltc_afe_crc15.h
 *
 * @brief  Header file to implement CRC15 for the LTC8611 AFE
 *
 * @date   2025-04-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup LTC6811
 * @brief    LTC6811 library
 * @{
 */

/**
 * @brief Initiliaze the CRC15 table for PEC
 * @details Used for fast CRC15 lookups
 */
void crc15_init_table(void);

/**
 * @brief Calculates CRC15 PEC value for given data
 *
 * @param data the data to calculate the PEC for
 * @param len length of the data
 * @return uint16_t Return PEC
 */
uint16_t crc15_calculate(uint8_t *data, size_t len);

/** @} */
