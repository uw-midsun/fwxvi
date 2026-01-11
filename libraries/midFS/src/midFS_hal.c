/************************************************************************************************
 * @file   midFS_hal.h
 *
 * @brief  Source file for midFS_hal, which is used to communicate between file system and HAL
 *
 * @date   2025-09-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "midFS_hal.h"

/* Intra-component Headers */
#ifdef ARCH_ARM

#define MIN(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b;      \
  })

#define LAST_PAGE (NUM_FLASH_PAGES - 1)

static PersistStorage storage = { 0U };

StatusCode fs_hal_init(uint8_t *fs_memory, size_t fs_memory_size) {
  StatusCode ret = flash_init();
  if (ret != STATUS_CODE_OK) {
    printf("flash_init() failed with exit code %u\r\n", ret);
    return STATUS_CODE_INCOMPLETE;
  }

  ret = persist_init(&storage, LAST_PAGE, fs_memory, fs_memory_size, true);
  if (ret != STATUS_CODE_OK) {
    printf("persist_init() failed with exit code %u\r\n", ret);
    return STATUS_CODE_INCOMPLETE;
  }

  return STATUS_CODE_OK;
}

StatusCode fs_hal_read(uint32_t address, uint8_t *buffer, size_t buffer_len) {
  if (buffer == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(buffer, (void *)address, buffer_len);
  return STATUS_CODE_OK;
}

StatusCode fs_hal_write(uint32_t address, uint8_t *buffer, size_t buffer_len) {
  if (buffer == NULL || buffer_len == 0) {
    return STATUS_CODE_INVALID_ARGS;
  }

  persist_commit(&storage);

  return STATUS_CODE_OK;
}

#endif
