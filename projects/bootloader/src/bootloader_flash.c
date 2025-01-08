/************************************************************************************************
 * @file   bootloader_flash.c
 *
 * @brief  Source file for the flash API in the bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "bootloader_flash.h"

BootloaderError boot_flash_write(uintptr_t address, uint8_t *buffer, size_t buffer_len) {
  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_flash_erase(uint8_t start_page, uint8_t num_pages) {
  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_flash_read(uintptr_t address, uint8_t *buffer, size_t buffer_len) {
  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_verify_flash_memory(void) {
  return BOOTLOADER_ERROR_NONE;
}
