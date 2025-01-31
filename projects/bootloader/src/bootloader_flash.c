/************************************************************************************************
 * @file   bootloader_flash.c
 *
 * @brief  Source file for the flash API in the bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "stm32l433xx.h"
#include "stm32l4xx_hal_flash.h"

/* Intra-component Headers */
#include "bootloader_flash.h"
#include "misc.h"

/* Symbols from linker-scripts */
extern uint32_t _flash_start;
extern uint32_t _flash_size;
extern uint32_t _flash_page_size;
extern uint32_t _bootloader_start;
extern uint32_t _application_start;
extern uint32_t _bootloader_size;
extern uint32_t _application_size;

static BootloaderError s_validate_address(uintptr_t address, size_t size) {
  /* Check memory alignment */
  if (address % BOOTLOADER_FLASH_WORD_SIZE != 0U || size % BOOTLOADER_FLASH_WORD_SIZE != 0U) {
    return BOOTLOADER_INVALID_ARGS;
  }

  /* Ensure address is in range */
  uint32_t flash_end = (uint32_t)&_flash_start + (uint32_t)&_flash_size;
  if (address < (uint32_t)&_flash_start || address + size > flash_end) {
    return BOOTLOADER_FLASH_WRITE_OUT_OF_BOUNDS;
  }

  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_flash_write(uintptr_t address, uint8_t *buffer, size_t buffer_len) {
  if (buffer == NULL) {
    return BOOTLOADER_INVALID_ARGS;
  }

  if (s_validate_address(address, buffer_len) == BOOTLOADER_ERROR_NONE) {
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    /* Program the flash double word by double word (64-bits) */
    for (size_t i = 0U; i < buffer_len; i += 8U) {
      uint64_t data = 0U;
      memcpy(&data, &buffer[i], MIN(8U, buffer_len - i));

      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + i, data) != HAL_OK) {
        HAL_FLASH_Lock();
        return BOOTLOADER_INTERNAL_ERR;
      }
    }
    HAL_FLASH_Lock();
  }

  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_flash_erase(uint8_t start_page, uint8_t num_pages) {
  if (start_page >= NUM_FLASH_PAGES || num_pages == 0U || start_page + num_pages > NUM_FLASH_PAGES) {
    return BOOTLOADER_INVALID_ARGS;
  }

  FLASH_EraseInitTypeDef erase_init = { .TypeErase = FLASH_TYPEERASE_PAGES, .Banks = FLASH_BANK_1, .Page = start_page, .NbPages = num_pages };

  uint32_t page_error = 0U;
  HAL_StatusTypeDef status;

  HAL_FLASH_Unlock();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
  status = HAL_FLASHEx_Erase(&erase_init, &page_error);
  HAL_FLASH_Lock();

  if (status != HAL_OK) {
    return BOOTLOADER_INTERNAL_ERR;
  }

  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_flash_read(uintptr_t address, uint8_t *buffer, size_t buffer_len) {
  if (buffer == NULL) {
    return BOOTLOADER_INVALID_ARGS;
  }

  if (s_validate_address(address, buffer_len) == BOOTLOADER_ERROR_NONE) {
    /* Direct memory read */
    memcpy(buffer, (void *)address, buffer_len);
  }

  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_verify_flash_memory(void) {
  uint8_t data[FLASH_SIZE];

  BootloaderError read_success = boot_flash_read(_flash_start, data, FLASH_SIZE);

  if (read_success == BOOTLOADER_ERROR_NONE) {
    for (int i = 0; i < FLASH_SIZE; i++) {
      if (data[i] != 0b11111111) {
        return BOOTLOADER_FLASH_MEMORY_VERIFY_FAILED;
      }
    }

    return BOOTLOADER_ERROR_NONE;
  }
}
