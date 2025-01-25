/************************************************************************************************
 * @file   flash.c
 *
 * @brief  Flash Library Source file
 *
 * @date   2024-11-05
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "log.h"
#include "semphr.h"
#include "status.h"
#include "stm32l433xx.h"
#include "stm32l4xx_hal_flash.h"

/* Intra-component Headers */
#include "flash.h"
#include "misc.h"

/* Symbols from linker-scripts */
// None are used in this case

#define FLASH_DEFAULT_FILENAME "Midsun_x86_flash"
#define FLASH_USER_ENV "MIDSUN_X86_FLASH_FILE"

static pthread_mutex_t s_flash_mutex;
// TODO: Mutex need to be deleted from memory at some point
static FILE *s_flash_fp = NULL;

#ifndef _flash_start
#define _flash_start 0x08000000
#endif

#ifndef _bootloader_size
#define _bootloader_size 0
#endif

#ifndef _flash_size
#define _flash_size 0x040000
#endif

static StatusCode s_validate_address(uintptr_t address, size_t size) {
  /* Check memory alignment */
  if (address % FLASH_MEMORY_ALIGNMENT != 0U || size % FLASH_MEMORY_ALIGNMENT != 0U) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Ensure adderss is in range */
  uint32_t flash_end = (uint32_t)_flash_start + (uint32_t)_flash_size;
  if (address < (uint32_t)_flash_start || address + size > flash_end) {
    return STATUS_CODE_OUT_OF_RANGE;
  }

  return STATUS_CODE_OK;
}

StatusCode flash_init() {
  char *flash_filename = getenv(FLASH_USER_ENV);

  if (flash_filename == NULL) {
    flash_filename = FLASH_DEFAULT_FILENAME;
  }
  LOG_DEBUG("Using flash file: %s\n", flash_filename);

  s_flash_fp = fopen(flash_filename, "r+b");
  if (s_flash_fp == NULL) {
    LOG_DEBUG("Setting up new flash file\n");
    s_flash_fp = fopen(flash_filename, "w+b");
    if (s_flash_fp == NULL) {
      LOG_DEBUG("Error: could not open flash file\n");
      exit(EXIT_FAILURE);
    }
    // TODO: Add erase code to simulate STM32 behavior
  }

  if (pthread_mutex_init(&s_flash_mutex, NULL) == 0) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_INTERNAL_ERROR;
  }
}

StatusCode flash_read(uintptr_t address, uint8_t *buffer, size_t buffer_len) {
  if (buffer == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  // TODO: Add timeout
  pthread_mutex_lock(&s_flash_mutex);

  if (s_validate_address(address, buffer_len) == STATUS_CODE_OK) {
    /* Direct memory read */
    memcpy(buffer, (void *)address, buffer_len);
  }

  pthread_mutex_unlock(&s_flash_mutex);
  return STATUS_CODE_OK;
}

StatusCode flash_write(uintptr_t address, uint8_t *buffer, size_t buffer_len) {
  if (buffer == NULL || s_validate_address(address, buffer_len) != STATUS_CODE_OK) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // TODO: Add timeout
  pthread_mutex_lock(&s_flash_mutex);

  // TODO: Add Write Code

  pthread_mutex_unlock(&s_flash_mutex);

  return STATUS_CODE_OK;
}

StatusCode flash_erase(uint8_t start_page, uint8_t num_pages) {
  if (start_page >= NUM_FLASH_PAGES || num_pages == 0U || start_page + num_pages > NUM_FLASH_PAGES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // TODO: Add timeout
  pthread_mutex_lock(&s_flash_mutex);

  // TODO: Add read code

  pthread_mutex_unlock(&s_flash_mutex);
  return STATUS_CODE_OK;
}
