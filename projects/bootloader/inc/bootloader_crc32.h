#pragma once

/************************************************************************************************
 * @file   bootloader_crc32.h
 *
 * @brief  Header file for the CRC32 calculations in the bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "bootloader_error.h"

/**
 * @defgroup bootloader
 * @brief    bootloader Firmware
 * @{
 */

/** @brief  Convert bytes to number of words for ARM32 system */
#define BYTES_TO_WORD(bytes) (bytes / 4)

/**
 * @brief   Initialize the CRC hardware component
 * @return  BOOTLOADER_ERROR_NONE if initialized succesfully
 */
BootloaderError boot_crc32_init(void);

/**
 * @brief   Calculate a CRC32 on a provided data buffer
 * @param   buffer Pointer to the data buffer to be checksummed
 * @param   buffer_len Length of the data buffer
 * @return  Calculated 32-bit CRC value
 */
uint32_t boot_crc32_calculate(const uint32_t *buffer, size_t buffer_len);

/**
 * @brief   Align a provided data buffer to the 4-byte boundary
 * @details This prevents hard faults when writing to flash memory, ensuring all our flash is 4-byte aligned
 * @param   buffer Pointer to the data buffer to be aligned
 * @param   buffer_len Length of the data buffer
 */
void boot_align_to_32bit_words(uint8_t *buffer, size_t *buffer_len);



/// test the last time
/** @} */
