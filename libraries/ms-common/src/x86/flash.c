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
#include "log.h"
#include "semphr.h"
#include "status.h"

/* Intra-component Headers */
#include "flash.h"

/* Symbols from linker-scripts */
// None are used in this case

#define FLASH_FILENAME "Midsun_x86_flash"

static pthread_mutex_t s_flash_mutex;
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
  char *flash_filename = FLASH_FILENAME;

  LOG_DEBUG("Using flash file: %s\n", flash_filename);

  // Opening the file or creating a new one if it does not exist
  s_flash_fp = fopen(flash_filename, "r+b");
  if (s_flash_fp == NULL) {
    LOG_DEBUG("Setting up new flash file...\n");
    s_flash_fp = fopen(flash_filename, "w+b");
    if (s_flash_fp == NULL) {
      LOG_DEBUG("Error: could not open flash file\n");
      exit(EXIT_FAILURE);
    }
    flash_erase(0, NUM_FLASH_PAGES);
  }

  // Initializing the mutex
  if (pthread_mutex_init(&s_flash_mutex, NULL) == 0) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_INTERNAL_ERROR;
  }
}

StatusCode flash_read(uintptr_t address, uint8_t *buffer, size_t buffer_len) {
  if (buffer == NULL || s_validate_address(address, buffer_len) != 0) {
    return STATUS_CODE_INVALID_ARGS;
  }

  pthread_mutex_lock(&s_flash_mutex);

  fseek(s_flash_fp, (intptr_t)address, SEEK_SET);
  size_t read = fread(buffer, 1, buffer_len, s_flash_fp);

  pthread_mutex_unlock(&s_flash_mutex);

  // Check if the number of objects read is less than buffer length to see if an error occured
  if (read < buffer_len) {
    return STATUS_CODE_INTERNAL_ERROR;
  } else {
    return STATUS_CODE_OK;
  }
}

StatusCode flash_write(uintptr_t address, uint8_t *buffer, size_t buffer_len) {
  if (buffer == NULL || s_validate_address(address, buffer_len) != STATUS_CODE_OK) {
    return STATUS_CODE_INVALID_ARGS;
  }

  pthread_mutex_lock(&s_flash_mutex);

  // Erase all memory to overwrite
  flash_erase(0, 128);

  // Seek to memeory address and write
  fseek(s_flash_fp, (intptr_t)address, SEEK_SET);
  fwrite(buffer, 1, buffer_len, s_flash_fp);
  fflush(s_flash_fp);

  pthread_mutex_unlock(&s_flash_mutex);

  return STATUS_CODE_OK;
}

StatusCode flash_erase(uint8_t start_page, uint8_t num_pages) {
  if (start_page >= NUM_FLASH_PAGES || num_pages == 0U || start_page + num_pages > NUM_FLASH_PAGES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  pthread_mutex_lock(&s_flash_mutex);

  size_t buffer_size = (num_pages - start_page) * FLASH_PAGE_SIZE;
  char *buffer = malloc(buffer_size);

  if (buffer == NULL) {
    pthread_mutex_unlock(&s_flash_mutex);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  memset(buffer, 0xFF, buffer_size);

  fseek(s_flash_fp, (intptr_t)FLASH_PAGE_TO_ADDR(start_page), SEEK_SET);
  fwrite(buffer, 1, buffer_size, s_flash_fp);
  fflush(s_flash_fp);

  free(buffer);
  pthread_mutex_unlock(&s_flash_mutex);

  return STATUS_CODE_OK;
}
