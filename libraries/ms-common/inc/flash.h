#pragma once

/************************************************************************************************
 * @file   flash.h
 *
 * @brief  Flash Library Header file
 *
 * @date   2024-11-05
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "status.h"

/**
 * @defgroup Flash
 * @brief    Flash library
 * @{
 */

/** @brief  Flash memory must be aligned to 4-bytes since STM32 is 32-bit device */
#define FLASH_MEMORY_ALIGNMENT 4U

/** @brief  When writing data, flash memory must be aligned to 8-bytes */
#define FLASH_MEMORY_WRITE_ALIGNMENT 8U

/** @brief  Flash memory begins at 0x0800 0000 as per ARM specification */
#define FLASH_BASE_ADDR (0x08000000U)

#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE 0x800U
#endif

/** @brief  Convert page number to a memory address */
#define FLASH_PAGE_TO_ADDR(page) (FLASH_BASE_ADDR + ((page) * FLASH_PAGE_SIZE))

/** @brief  Convert memory address to a page number */
#define FLASH_ADDR_TO_PAGE(addr) (((addr) - FLASH_BASE_ADDR) / FLASH_PAGE_SIZE)

/** @brief Maximum time permitted for a flash transaction */
#define FLASH_TIMEOUT_MS 100U

/** @brief  Total number of flash pages in the STM32L433CCU6 */
#define NUM_FLASH_PAGES 128U

/** @brief  Last flash page number in the STM32L433CCU6 */
#define FLASH_LAST_PAGE_NUM (NUM_FLASH_PAGES - 1U)

/**
 * @brief  Application region bounds from the linker map
 * @details Defined by the generated map only when an app is built to run under the bootloader,
 *          declared weak so a standalone or legacy build, where the map omits them, links with the
 *          addresses resolving to zero instead of failing
 */
extern uint32_t _app_start __attribute__((weak));
extern uint32_t _app_size __attribute__((weak));

/**
 * @brief   Highest flash page an application may use for persistent storage
 * @details The bootloader reserves the pages above the app region for its BootConfig, so an app
 *          running under it must keep its storage inside the app region, one page below where the
 *          reserved region begins, the page is derived from the linker geometry at runtime, so
 *          there is one source of truth and no build time flag, a standalone or legacy app (no app
 *          region symbols) owns the whole flash and uses the chip's last page
 * @return  Page number to hand to persist_init or flash_erase for application storage
 */
static inline uint8_t flash_app_storage_page(void) {
  uintptr_t app_start = (uintptr_t)&_app_start;
  uintptr_t app_size = (uintptr_t)&_app_size;
  if (app_size != 0U) {
    return (uint8_t)(FLASH_ADDR_TO_PAGE(app_start + app_size) - 1U);
  }
  return FLASH_LAST_PAGE_NUM;
}

/**
 * @brief   Initialize flash API
 * @details This initializes the mutex used in the flash API
 */
StatusCode flash_init();

/**
 * @brief   Read from the flash memory and store data into a buffer
 * @param   address Memory address to read from. This value MUST be 4-byte aligned
 * @param   buffer Pointer to the buffer to store data
 * @param   buffer_len Length of the buffer. This value MUST be 4-byte aligned
 * @return  STATUS_CODE_OK if flash memory was read successfully
 *          STATUS_CODE_OUT_OF_RANGE if address is out of range
 *          STATUS_CODE_INVALID_ARGS if address or read bytes is not aligned
 */
StatusCode flash_read(uintptr_t address, uint8_t *buffer, size_t buffer_len);

/**
 * @brief   Write a buffer of data into the flash memory
 * @details This does not rewrite the flash memory. Once written, the data must be cleared
 *          by erasing the entire page
 * @param   address Memory address to store the buffer. This value MUST be 4-byte aligned
 * @param   buffer Pointer to the buffer of data to store
 * @param   buffer_len Length of the buffer to write. This MUST also be 4-byte aligned
 * @return  STATUS_CODE_OK if flash memory was written successfully
 *          STATUS_CODE_OUT_OF_RANGE if address is out of range
 *          STATUS_CODE_INTERNAL_ERROR if flash write failed
 */
StatusCode flash_write(uintptr_t address, uint8_t *buffer, size_t buffer_len);

/**
 * @brief   Erase pages of flash memory
 * @param   start_page First page to erase
 * @param   num_pages Number of pages to erase
 * @return  STATUS_CODE_OK if flash memory was erased successfully
 *          STATUS_CODE_INVALID_ARGS if incorrect arguments were passed in
 *          STATUS_CODE_INTERNAL_ERROR if flash erase failed
 */
StatusCode flash_erase(uint8_t start_page, uint8_t num_pages);

/** @} */
