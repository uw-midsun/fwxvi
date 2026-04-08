#pragma once

/************************************************************************************************
 * @file   crc8.h
 *
 * @brief  Portable CRC-8 calculation (polynomial 0x07, init 0xFF)
 *
 * @date   2026-04-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>
#include <stddef.h>

/**
 * @defgroup CRC8
 * @brief    CRC-8 library
 * @{
 */

/**
 * @brief   Compute CRC-8 over a byte buffer
 * @param   data  Pointer to byte array
 * @param   len   Number of bytes
 * @return  CRC-8 checksum (polynomial 0x07, init 0xFF)
 */
uint8_t crc8_calculate(const uint8_t *data, size_t len);

/** @} */
