#pragma once

/************************************************************************************************
 * @file    fota_flash.h
 *
 * @brief   Fota Flash Header File
 *
 * @date    2025-04-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_error.h"

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

/** @brief 256KB of memory / 2KB per page */
#define NUM_FLASH_PAGES 128U

/** @brief  2KB per flash page as per datasheet */
#define FOTA_PAGE_BYTES ((uint32_t)0x800U)

/** @brief  4KB max per ping */
#define FOTA_PING_BYTES ((uint32_t)0x1000U)

/** @brief  ARM32 Word size is 4 bytes */
#define FOTA_FLASH_WORD_SIZE 4U

/**
 * @brief   Convert memory address to page number
 * @param   addr Address to be converted to page number
 * @return  Page number
 */
#define FOTA_ADDR_TO_PAGE(addr) (((uintptr_t)(addr) - (uintptr_t)APPLICATION_START_ADDRESS) / FOTA_PAGE_BYTES)

/**
 * @brief   Convert page number to memory address
 * @param   page Page number
 * @return  32-bit address
 */
#define FOTA_PAGE_TO_ADDR(page) ((uintptr_t)(page) * (uintptr_t)FOTA_PAGE_BYTES + (uintptr_t)APPLICATION_START_ADDRESS)

/**
 * @brief   Write to flash memory
 * @param   address Base memory address to write to
 * @param   buffer Pointer to the data buffer
 * @param   buffer_len Length of the data buffer
 * @return  FOTA_ERROR_SUCCESS if data is written succesfully
 *          FOTA_ERROR_FLASH_WRITE_FAILED if HAL flash fails
 *          FOTA_ERROR_FLASH_WRITE_OUT_OF_BOUNDS if address is out of bounds
 *          FOTA_ERROR_FLASH_WRITE_NOT_ALIGNED if data is not aligned
 */
FotaError fota_flash_write(uintptr_t address, uint8_t *buffer, uint32_t buffer_len);

/**
 * @brief   Erase some number of flash pages
 * @param   start_page Initial page number to erase
 * @param   num_pages Number of pages to erase
 * @return  FOTA_ERROR_SUCCESS if data is erased succesfully
 *          FOTA_ERROR_FLASH_ERASE if page is out of bounds OR erasing failed
 */
FotaError fota_flash_erase(uint8_t start_page, uint8_t num_pages);

/**
 * @brief   Read from flash memory
 * @param   address Base memory address to read from
 * @param   buffer Pointer to the data buffer
 * @param   buffer_len Length of the data buffer
 * @return  FOTA_ERROR_SUCCESS if data is read succesfully
 *          FOTA_ERROR_FLASH_READ_FAILED if data read failed
 */
FotaError fota_flash_read(uintptr_t address, uint8_t *buffer, uint32_t buffer_len);

/**
 * @brief   Verify that the flash memory is not erased
 * @details This checks the entire application flash memory to ensure it is not entirely erased
 * @return  FOTA_ERROR_SUCCESS if the memomry is valid
 *          FOTA_ERROR_FLASH_ERR if the memory is corrupt
 */
FotaError fota_verify_flash_memory(void);

/** @} */
