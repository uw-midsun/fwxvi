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
#ifdef MS_PLATFORM_ARM
#include "stm32l4xx_hal_flash.h"
#endif

/**
 * @defgroup Flash
 * @brief    Flash library
 * @{
 */

/* Ctrl c + v from stm32l4xx_hal_flash.h (workaround to not including hal for x86 builds) */
#ifdef MS_PLATFORM_X86
#if defined (STM32L4P5xx) || defined (STM32L4Q5xx) || defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define FLASH_PAGE_SIZE                    ((uint32_t)0x1000)
#define FLASH_PAGE_SIZE_128_BITS           ((uint32_t)0x2000)
#else
#define FLASH_PAGE_SIZE                    ((uint32_t)0x800)
#endif
#endif

/** @brief  Flash memory must be aligned to 4-bytes since STM32 is 32-bit device */
#define FLASH_MEMORY_ALIGNMENT 4U

/** @brief  When writing data, flash memory must be aligned to 8-bytes */
#define FLASH_MEMORY_WRITE_ALIGNMENT 8U

/** @brief  Flash memory begins at 0x0800 0000 as per ARM specification */
#define FLASH_BASE_ADDR (0x08000000U)

#ifndef FLASH_PAGE_SIZE
/** @brief  Flash page size is 2Kbyte as per table 63. in STM32L433xx datasheet */
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
