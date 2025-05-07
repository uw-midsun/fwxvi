/************************************************************************************************
 * @file   bootloader_crc32.c
 *
 * @brief  Source file for the CRC32 calculations in the bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "bootloader_crc32.h"

BootloaderError boot_crc32_init(void) {
  return BOOTLOADER_ERROR_NONE;
}

uint32_t boot_crc32_calculate(const uint32_t *buffer, size_t buffer_len) {
  return 0;
}

void boot_align_to_32bit_words(uint8_t *buffer, size_t *buffer_len) {}
