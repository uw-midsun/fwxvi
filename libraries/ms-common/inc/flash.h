#pragma once

/************************************************************************************************
 * flash.h
 *
 * Flash Library Header file
 *
 * Created: 2024-11-05
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "status.h"

#define FLASH_MEMORY_ALIGNMENT 4U

#define FLASH_BASE_ADDR (0x08000000U)

#define FLASH_PAGE_TO_ADDR(page) (FLASH_BASE_ADDR + ((page) * FLASH_PAGE_SIZE))
#define FLASH_ADDR_TO_PAGE(addr) (((addr) - FLASH_BASE_ADDR) / FLASH_PAGE_SIZE)

#define NUM_FLASH_PAGES 128U

/**
 * @brief   Read from the flash memory and store data into a buffer
 * @param   address Memory address to read from. This value MUST be 4-byte aligned
 * @param   buffer Pointer to the buffer to store data
 * @param   buffer_len Length of the buffer. This value MUST be 4-byte aligned
 * @return  STATUS_CODE_OK if flash memory was read succesfully
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
 * @return  STATUS_CODE_OK if flash memory was written succesfully
 *          STATUS_CODE_OUT_OF_RANGE if address is out of range
 *          STATUS_CODE_INTERNAL_ERROR if flash write failed
 */
StatusCode flash_write(uintptr_t address, uint8_t *buffer, size_t buffer_len);

/**
 * @brief   Erase pages of flash memory
 * @param   start_page First page to erase
 * @param   num_pages Number of pages to erase
 * @return  STATUS_CODE_OK if flash memory was erased succesfully
 *          STATUS_CODE_INVALID_ARGS if incorrect arguments were passed in
 *          STATUS_CODE_INTERNAL_ERROR if flash erase failed
 */
StatusCode flash_erase(uint8_t start_page, uint8_t num_pages);
