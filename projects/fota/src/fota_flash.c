/************************************************************************************************
 * @file    fota_flash.c
 *
 * @brief   Fota Flash Source File
 *
 * @date    2025-04-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "misc.h"
#include "stm32l433xx.h"
#include "stm32l4xx_hal_flash.h"

/* Intra-component Headers */
#include "fota_error.h"
#include "fota_flash.h"
#include "fota_memory_map.h"

static FotaError s_validate_address(uint32_t address, size_t size) {
  /* Check memory alignment */
  if (address % FOTA_FLASH_WORD_SIZE != 0U || size % FOTA_FLASH_WORD_SIZE != 0U) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  /* Ensure address is in range */
  uint32_t flash_end = FLASH_START_ADDRESS_LINKERSCRIPT + FLASH_SIZE;
  if (address < FLASH_START_ADDRESS_LINKERSCRIPT || address + size > flash_end) {
    return FOTA_ERROR_FLASH_WRITE_OUT_OF_BOUNDS;
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_flash_write(uint32_t address, uint8_t *buffer, size_t buffer_len) {
  if (buffer == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  if (s_validate_address(address, buffer_len) == FOTA_ERROR_SUCCESS) {
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    /* Program the flash double word by double word (64-bits) */
    for (size_t i = 0U; i < buffer_len; i += 8U) {
      uint64_t data = 0U;
      memcpy(&data, &buffer[i], MIN(8U, buffer_len - i));

      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + i, data) != HAL_OK) {
        HAL_FLASH_Lock();
        return FOTA_ERROR_FLASH_WRITE_FAILED;
      }
    }
    HAL_FLASH_Lock();
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_flash_erase(uint8_t start_page, uint8_t num_pages) {
  if (start_page >= NUM_FLASH_PAGES || num_pages == 0U || start_page + num_pages > NUM_FLASH_PAGES) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  FLASH_EraseInitTypeDef erase_init = { .TypeErase = FLASH_TYPEERASE_PAGES, .Banks = FLASH_BANK_1, .Page = start_page, .NbPages = num_pages };

  uint32_t page_error = 0U;
  HAL_StatusTypeDef status;

  HAL_FLASH_Unlock();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
  status = HAL_FLASHEx_Erase(&erase_init, &page_error);
  HAL_FLASH_Lock();

  if (status != HAL_OK) {
    return FOTA_ERROR_FLASH_ERASE;
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_flash_read(uint32_t address, uint8_t *buffer, size_t buffer_len) {
  if (buffer == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  if (s_validate_address(address, buffer_len) == FOTA_ERROR_SUCCESS) {
    /* Direct memory read */
    memcpy(buffer, (void *)address, buffer_len);
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_verify_flash_memory() {
  volatile uint32_t *flash_pointer = (volatile uint32_t *) FLASH_START_ADDRESS_LINKERSCRIPT;
  uint32_t size_in_words = APPLICATION_SIZE / sizeof(uint32_t);

  for (uint32_t i = 0; i < size_in_words; i++) {
    if (flash_pointer[i] != 0xFFFFFFFFU) {
      return FOTA_ERROR_SUCCESS;
    }
  }

  return FOTA_ERROR_FLASH_VERIFICATION_FAILED;
}


//Not sure if this is needed since the flash memory is already verified in fota_verify_flash_memory
// but keeping it for now as it might be useful in the future (e.g. for verifying a specific application section)
FotaError fota_verify_flash_memory_application() {
  volatile uint32_t *flash_pointer = (volatile uint32_t *) APPLICATION_SIZE;
  uint32_t size_in_words = APPLICATION_SIZE / sizeof(uint32_t);

  for (uint32_t i = 0; i < size_in_words; i++) {
    if (flash_pointer[i] != 0xFFFFFFFFU) {
      return FOTA_ERROR_SUCCESS;
    }
  }

  return FOTA_ERROR_FLASH_VERIFICATION_FAILED;
}
