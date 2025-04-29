#pragma once

/************************************************************************************************
 * @file   persist.h
 *
 * @brief  Persist API Header file
 *
 * @date   2025-03-10
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "flash.h"
#include "status.h"

/**
 * @defgroup persist
 * @brief    persist Firmware
 * @{
 */

// Commit data every second if dirty
#define PERSIST_COMMIT_TIMEOUT_MS 1000

typedef struct PersistStorage {
  void *blob;
  size_t blob_size;
  uintptr_t flash_addr;
  uintptr_t prev_flash_addr;
  uint32_t prev_hash;
  uint8_t page;
} PersistStorage;

// Attempt to load stored data into the provided blob and retains the blob to
// commit periodically Reserves the entire flash page for the persistance layer
// instance Note that the blob must be a multiple of FLASH_WRITE_BYTES and must
// persist If |overwrite| is true, the persist layer overwrites invalid blobs.
// Otherwise, it fails.
StatusCode persist_init(PersistStorage *persist, uint8_t page, void *blob, size_t blob_size, bool overwrite);

// Force a data commit - this should be avoided if possible.
StatusCode persist_commit(PersistStorage *persist);

/** @} */
