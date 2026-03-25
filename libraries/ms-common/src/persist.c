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

/* Erased flash defaults to all 1's */
#define PERSIST_VALID_MARKER 0xFFFFFFFFU
#define PERSIST_INVALID_SIZE 0xFFFFFFFFU
#define PERSIST_INVALID_ADDR UINTPTR_MAX
#define PERSIST_BASE_ADDR FLASH_PAGE_TO_ADDR(persist->page)
#define PERSIST_END_ADDR (PERSIST_BASE_ADDR + FLASH_PAGE_SIZE)

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
  } else if (persist == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  persist->blob = blob;
  persist->blob_size = blob_size;
  persist->prev_flash_addr = PERSIST_INVALID_ADDR;
  persist->page = page;

  /* Load stored data */
  PersistHeader header = { .marker = PERSIST_VALID_MARKER, .size_bytes = 0 };

  persist->flash_addr = PERSIST_BASE_ADDR;

  /**
   * We want to exit in three scenarios:
   * - If the page has been erased and there are no valid sections
   *    - Use the base address. Marker == 0xFFFFFFFF, Size == 0xFFFFFFFF
   * - If this is an invalid section of memory
   *    - Increment the address by (header + blob). Marker != 0xFFFFFFFF, Size != 0xFFFFFFFF
   * - If this is a valid section
   *    - Use the current address. Marker == 0xFFFFFFFF, Size != 0xFFFFFFFF
   */
  do {
    /* Read flash memory into the header while theres no valid marker */
    status_ok_or_return(flash_read(persist->flash_addr, (uint8_t *)&header, sizeof(header)));

    if (header.marker != PERSIST_VALID_MARKER) {
      /* If it is an invalid marker, we increment the flash base address and continue */
      persist->flash_addr += sizeof(header) + header.size_bytes;
    }

    if (persist->flash_addr >= PERSIST_END_ADDR) {
      /* If zero valid sections remain, erase the entire page */
      status_ok_or_return(flash_erase(persist->page, 1));
      persist->flash_addr = PERSIST_BASE_ADDR;
    }
  } while (header.marker != PERSIST_VALID_MARKER);

  if (header.size_bytes == PERSIST_INVALID_SIZE) {
    /* If there was no valid header found, we will just commit the data to the base address */
    persist_commit(persist);
  } else if (header.size_bytes != persist->blob_size) {
    /* If the persist memory region is not the same as the blob size, we invalidate the current section */
    if (overwrite) {
      /* Overwrite the existing memory */
      persist->prev_flash_addr = persist->flash_addr;
      persist->flash_addr += sizeof(header) + header.size_bytes;
      persist_commit(persist);
    } else {
      return STATUS_CODE_INTERNAL_ERROR;
    }
  } else {
    /* Found a valid section of same size blob */
    LOG_DEBUG("Found valid section at 0x%" PRIx32 " (0x%" PRIx32 " bytes), loading data\n", (uint32_t)persist->flash_addr, (uint32_t)header.size_bytes);

    /* Load the blob data from flash memory*/
    status_ok_or_return(flash_read(persist->flash_addr + sizeof(header), (uint8_t *)persist->blob, persist->blob_size));

    /* Increment flash_addr to the next new section */
    persist->prev_flash_addr = persist->flash_addr;
    persist->flash_addr += sizeof(header) + header.size_bytes;
  }

  s_context = persist;

  return STATUS_CODE_OK;
}

StatusCode persist_commit(PersistStorage *persist) {
  if (persist == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Mark previous section as invalid */
  if (persist->prev_flash_addr != PERSIST_INVALID_ADDR) {
    uint32_t invalid = 0U;
    flash_write(persist->prev_flash_addr, (uint8_t *)&invalid, sizeof(invalid));
  }

  /* Check if we're overrunning the page */
  if (persist->flash_addr + sizeof(PersistHeader) + persist->blob_size >= PERSIST_END_ADDR) {
    flash_erase(persist->page, 1U);
    persist->flash_addr = PERSIST_BASE_ADDR;
  }

  /* Write header */
  PersistHeader header = { .marker = PERSIST_VALID_MARKER, .size_bytes = persist->blob_size };
  status_ok_or_return(flash_write(persist->flash_addr, (uint8_t *)&header, sizeof(header)));

  /* Write persist blob */
  status_ok_or_return(flash_write(persist->flash_addr + sizeof(header), (uint8_t *)persist->blob, persist->blob_size));

  persist->prev_flash_addr = persist->flash_addr;
  persist->flash_addr += sizeof(header) + persist->blob_size;

  if (persist->flash_addr % FLASH_MEMORY_WRITE_ALIGNMENT != 0U) {
    persist->flash_addr += FLASH_MEMORY_WRITE_ALIGNMENT - (persist->flash_addr % FLASH_MEMORY_WRITE_ALIGNMENT);
  }

  return STATUS_CODE_OK;
}
