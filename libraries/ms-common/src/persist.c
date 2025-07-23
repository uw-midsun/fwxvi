/************************************************************************************************
 * @file    persist.c
 *
 * @brief   Persist
 *
 * @date    2025-04-29
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <inttypes.h>

/* Inter-component Headers */
#include "flash.h"
#include "log.h"
#include "persist.h"

/* Intra-component Headers */
#include "status.h"

// Erased flash defaults to all 1's
#define PERSIST_VALID_MARKER 0xFFFFFFFF
#define PERSIST_INVALID_SIZE 0xFFFFFFFF
#define PERSIST_INVALID_ADDR UINTPTR_MAX
#define PERSIST_BASE_ADDR FLASH_PAGE_TO_ADDR(persist->page)
#define PERSIST_END_ADDR (PERSIST_BASE_ADDR + FLASH_PAGE_SIZE)

int id;

static PersistStorage *s_context;

typedef struct PersistHeader {
  uint32_t marker;
  uint32_t size_bytes;
} PersistHeader;

StatusCode persist_init(PersistStorage *persist, uint8_t page, void *blob, size_t blob_size, bool overwrite) {
  if (blob_size > FLASH_PAGE_SIZE || page >= NUM_FLASH_PAGES) {
    return STATUS_CODE_OUT_OF_RANGE;
  } else if (blob_size % FLASH_MEMORY_ALIGNMENT != 0) {
    return STATUS_CODE_INVALID_ARGS;
  }

  persist->blob = blob;
  persist->blob_size = blob_size;
  persist->prev_flash_addr = PERSIST_INVALID_ADDR;
  persist->page = page;

  // Load stored data
  PersistHeader header = {
    .marker = PERSIST_VALID_MARKER,  //
    .size_bytes = 0                  //
  };
  persist->flash_addr = PERSIST_BASE_ADDR;

  // Essentially, we want to exit in three scenarios:
  // * The page has been erased and there are no valid sections. Use the base address.
  //   Marker == 0xFFFFFFFF, Size == 0xFFFFFFFF
  // * This is an invalid section. Increment the address by (header + blob).
  //   Marker != 0xFFFFFFFF, Size != 0xFFFFFFFF
  // * This is a valid section. Use this address.
  //   Marker == 0xFFFFFFFF, Size != 0xFFFFFFFF
  do {
    status_ok_or_return(flash_read(persist->flash_addr, (uint8_t *)&header, sizeof(header)));
    if (header.marker != PERSIST_VALID_MARKER) {
      persist->flash_addr += sizeof(header) + header.size_bytes;
    }

    if (persist->flash_addr >= PERSIST_END_ADDR) {
      // Somehow had zero valid sections remaining - erase page
      LOG_DEBUG("Somehow started with a full invalid page - erasing page\n");
      status_ok_or_return(flash_erase(persist->page, 1));
      persist->flash_addr = PERSIST_BASE_ADDR;
    }
  } while (header.marker != PERSIST_VALID_MARKER);

  if (header.size_bytes == PERSIST_INVALID_SIZE) {
    LOG_DEBUG("No valid sections found! New persist data will live at 0x%" PRIx32 "\n", (uint32_t)persist->flash_addr);
    persist_commit(persist);
  } else if (header.size_bytes != persist->blob_size) {
    if (overwrite) {
      LOG_DEBUG("Mismatched blob sizes! Invalidating old section\n");
      persist->prev_flash_addr = persist->flash_addr;
      persist->flash_addr += sizeof(header) + header.size_bytes;
      persist_commit(persist);
    } else {
      LOG_DEBUG("Mismatched blob sizes! Failed to init persist layer\n");
      return STATUS_CODE_INTERNAL_ERROR;
    }
  } else {
    LOG_DEBUG("Found valid section at 0x%" PRIx32 " (0x%" PRIx32 " bytes), loading data\n", (uint32_t)persist->flash_addr, (uint32_t)header.size_bytes);
    StatusCode ret = flash_read(persist->flash_addr + sizeof(header), (uint8_t *)persist->blob, persist->blob_size);
    status_ok_or_return(ret);

    // Calculate valid section's hash
    // persist->prev_hash = crc32_arr((const uint8_t *)persist->blob, persist->blob_size);

    // Increment flash_addr to the next new section
    persist->prev_flash_addr = persist->flash_addr;
    persist->flash_addr += sizeof(header) + header.size_bytes;
  }
  s_context = persist;
  return STATUS_CODE_OK;
}

StatusCode persist_commit(PersistStorage *persist) {
  // Mark previous section as invalid
  if (persist->prev_flash_addr != PERSIST_INVALID_ADDR) {
    uint32_t invalid = 0;
    flash_write(persist->prev_flash_addr, (uint8_t *)&invalid, sizeof(invalid));
  }

  // Check if we're overrunning the page
  if (persist->flash_addr + sizeof(PersistHeader) + persist->blob_size >= PERSIST_END_ADDR) {
    flash_erase(persist->page, 1);
    persist->flash_addr = PERSIST_BASE_ADDR;
  }

  // Write persist blob size, skipping the marker
  PersistHeader header = { .size_bytes = persist->blob_size };
  LOG_DEBUG("Committing persistance layer to 0x%" PRIx32 "\n", (uint32_t)persist->flash_addr);
  StatusCode ret = flash_write(persist->flash_addr + sizeof(header.marker), (uint8_t *)&header.size_bytes, sizeof(header.size_bytes));
  status_ok_or_return(ret);

  // Write persist blob
  ret = flash_write(persist->flash_addr + sizeof(header), (uint8_t *)persist->blob, persist->blob_size);
  status_ok_or_return(ret);

  persist->prev_flash_addr = persist->flash_addr;
  persist->flash_addr += sizeof(header) + persist->blob_size;

  return STATUS_CODE_OK;
}
