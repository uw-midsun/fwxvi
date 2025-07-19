/************************************************************************************************
 * @file    fota_flash.c
 *
 * @brief   FOTA Flash Source File
 *
 * @date    2025-04-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_error.h"
#include "fota_flash.h"
#include "fota_memory_map.h"

#define SIMULATED_FLASH_BASE_ADDRESS FLASH_START_ADDRESS_LINKERSCRIPT
#define SIMULATED_FLASH_SIZE FLASH_SIZE_LINKERSCRIPT
#define SIMULATED_FLASH_PAGE_SIZE FLASH_PAGE_SIZE_LINKERSCRIPT

#define FOTA_FLASH_WORD_SIZE 4U

static uint8_t simulated_flash[SIMULATED_FLASH_SIZE];

static uint32_t to_flash_offset(uint32_t address) {
  return address - SIMULATED_FLASH_BASE_ADDRESS;
}

static FotaError s_validate_address(uint32_t address, size_t size) {
  if ((address % FOTA_FLASH_WORD_SIZE) != 0U || (size % FOTA_FLASH_WORD_SIZE) != 0U) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  uint32_t end = address + size;
  if (address < SIMULATED_FLASH_BASE_ADDRESS || end > (SIMULATED_FLASH_BASE_ADDRESS + SIMULATED_FLASH_SIZE)) {
    return FOTA_ERROR_FLASH_WRITE_OUT_OF_BOUNDS;
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_flash_write(uint32_t address, uint8_t *buffer, size_t buffer_len) {
  if (buffer == NULL) return FOTA_ERROR_INVALID_ARGS;

  if (s_validate_address(address, buffer_len) != FOTA_ERROR_SUCCESS) {
    return FOTA_ERROR_FLASH_WRITE_OUT_OF_BOUNDS;
  }

  uint32_t offset = to_flash_offset(address);

  for (size_t i = 0; i < buffer_len; ++i) {
    if ((simulated_flash[offset + i] & buffer[i]) != buffer[i]) {
      return FOTA_ERROR_FLASH_WRITE_FAILED;
    }
    simulated_flash[offset + i] &= buffer[i];
  }

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_flash_read(uint32_t address, uint8_t *buffer, size_t buffer_len) {
  if (buffer == NULL) return FOTA_ERROR_INVALID_ARGS;

  if (s_validate_address(address, buffer_len) != FOTA_ERROR_SUCCESS) {
    return FOTA_ERROR_FLASH_WRITE_OUT_OF_BOUNDS;
  }

  uint32_t offset = to_flash_offset(address);
  memcpy(buffer, &simulated_flash[offset], buffer_len);

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_flash_erase(uint8_t start_page, uint8_t num_pages) {
  if (num_pages == 0U || (start_page + num_pages) > (SIMULATED_FLASH_SIZE / SIMULATED_FLASH_PAGE_SIZE)) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  uint32_t start_offset = start_page * SIMULATED_FLASH_PAGE_SIZE;
  uint32_t length = num_pages * SIMULATED_FLASH_PAGE_SIZE;

  memset(&simulated_flash[start_offset], 0xFFU, length);

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_verify_flash_memory(uintptr_t start_addr, size_t size_bytes) {
  if (start_addr % FOTA_FLASH_WORD_SIZE != 0U || size_bytes % FOTA_FLASH_WORD_SIZE != 0U) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  if (start_addr < FLASH_START_ADDRESS_LINKERSCRIPT || (start_addr + size_bytes) > (FLASH_START_ADDRESS_LINKERSCRIPT + FLASH_SIZE_LINKERSCRIPT)) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  size_t offset = start_addr - FLASH_START_ADDRESS_LINKERSCRIPT;
  uint32_t size_in_words = size_bytes / FOTA_FLASH_WORD_SIZE;

  for (uint32_t i = 0U; i < size_in_words; i++) {
    uint32_t word;
    memcpy(&word, &simulated_flash[offset + i * FOTA_FLASH_WORD_SIZE], FOTA_FLASH_WORD_SIZE);

    if (word != 0xFFFFFFFFU) {
      return FOTA_ERROR_SUCCESS;
    }
  }

  return FOTA_ERROR_FLASH_VERIFICATION_FAILED;
}
