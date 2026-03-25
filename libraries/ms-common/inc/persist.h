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

/* Commit data every second if dirty */
#define PERSIST_COMMIT_TIMEOUT_MS 1000

typedef struct {
  void *blob;                /**< Pointer to the RAM buffer/struct being persisted */
  size_t blob_size;          /**< Size of the buffer/struct being persisted*/
  uintptr_t flash_addr;      /**< Current write pointer within a reserved page */
  uintptr_t prev_flash_addr; /**< Address of the most recent valid record (for invalidation) */
  uint8_t page;              /**< Page index reserved for this blob */
} PersistStorage;

/**
 * @brief   Attempt to load stored data into the provided blob
 * @details Reserves the entire flash page for the persistance layer instance
 *          Note that the blob must be a multiple of FLASH_WRITE_BYTES and must
 *          persist If |overwrite| is true. the persist layer overwrites invalid blobs
 * @param   persist Pointer to the persist storage instance
 * @param   page Flash page selection (Max: NUM_FLASH_PAGES) for persist to initialize on
 * @param   blob Pointer to the struct/blob you want to save or load from flash memory
 * @param   blob_size Size of the struct/blob you want to save or load from flash memory
 * @param   overwrite Boolean flag to overwrite existing blobs
 * @return  STATUS_CODE_OK when persist is initialized successfully
 *          STATUS_CODE_OUT_OF_RANGE if an invalid flash page is selected
 *          STATUS_CODE_INVALID_ARGS if invalid parameters are passed
 */
StatusCode persist_init(PersistStorage *persist, uint8_t page, void *blob, size_t blob_size, bool overwrite);

/**
 * @brief   Force a persist data commit
 * @details Must be called after persist_init is called
 * @param   persist Pointer to the persist storage instance
 * @return  STATUS_CODE_OK if committed successfully
 *          STATUS_CODE_INVALID_ARGS if invalid parameters are passed
 */
StatusCode persist_commit(PersistStorage *persist);

/** @} */
