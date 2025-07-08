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

FotaError fota_verify_flash_memory() {
  for (size_t i = 0; i < APPLICATION_SIZE; i += sizeof(uint32_t)) {
    uint32_t word;
    memcpy(&word, &simulated_flash[i], sizeof(uint32_t));
    if (word != 0xFFFFFFFFU) {
      return FOTA_ERROR_SUCCESS;
    }
  }
  return FOTA_ERROR_FLASH_VERIFICATION_FAILED;
}

FotaError fota_verify_flash_memory_application() {
  uint32_t app_offset = APPLICATION_START_ADDRESS - SIMULATED_FLASH_BASE_ADDRESS;

  for (size_t i = 0; i < APPLICATION_SIZE; i += sizeof(uint32_t)) {
    uint32_t word;
    memcpy(&word, &simulated_flash[app_offset + i], sizeof(uint32_t));
    if (word != 0xFFFFFFFFU) {
      return FOTA_ERROR_SUCCESS;
    }
  }

  return FOTA_ERROR_FLASH_VERIFICATION_FAILED;
}
