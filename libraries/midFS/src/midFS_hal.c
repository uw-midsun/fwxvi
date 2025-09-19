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

#define MIN(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b;      \
  })

FsStatus fs_hal_read(uint32_t address, uint8_t *buffer, size_t buffer_len) {
  if (buffer == NULL) {
    return FS_STATUS_INVALID_ARGS;
  }

  memcpy(buffer, (void *)address, buffer_len);
  return FS_STATUS_OK;
}

FsStatus fs_hal_write(uint32_t address, uint8_t *buffer, size_t buffer_len) {
  if (buffer == NULL) {
    return FS_STATUS_INVALID_ARGS;
  }

  HAL_FLASH_Unlock();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

  /* Program the flash double word by double word (64-bits) */
  for (size_t i = 0U; i < buffer_len; i += 8U) {
    uint64_t data = 0U;
    memcpy(&data, &buffer[i], MIN(8U, buffer_len - i));

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + i, data) != HAL_OK) {
      HAL_FLASH_Lock();
      return FS_STATUS_INCOMPLETE;
    }
  }
  HAL_FLASH_Lock();

  return FS_STATUS_OK;
}
