#pragma once

/************************************************************************************************
 * @file   bootloader_flash.h
 *
 * @brief  Header file for the flash API in the bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "bootloader.h"
#include "bootloader_error.h"

/**
 * @defgroup bootloader
 * @brief    bootloader Firmware
 * @{
 */

/** @brief 256KB of memory / 2KB per page */
#define NUM_FLASH_PAGES 128U

/** @brief  2KB per flash page as per datasheet */
#define BOOTLOADER_PAGE_BYTES ((size_t)0x800)

/** @brief  4KB max per ping */
#define BOOTLOADER_PING_BYTES ((size_t)0x1000) //

/** @brief  ARM32 Word size is 4 bytes */
#define BOOTLOADER_FLASH_WORD_SIZE 4U

/**
 * @brief   Convert memory address to page number
 * @param   addr Address to be converted to page number
 * @return  Page number
 */
#define BOOTLOADER_ADDR_TO_PAGE(addr) (((uintptr_t)(addr) - (uintptr_t)APP_ACTIVE_START_ADDRESS) / BOOTLOADER_PAGE_BYTES)

/**
 * @brief   Convert page number to memory address
 * @param   page Page number
 * @return  32-bit address
 */
#define BOOTLOADER_PAGE_TO_ADDR(page) ((uintptr_t)(page) * (uintptr_t)BOOTLOADER_PAGE_BYTES + (uintptr_t)APP_ACTIVE_START_ADDRESS)

/**
 * @brief   Write to flash memory
 * @param   address Base memory address to write to
 * @param   buffer Pointer to the data buffer
 * @param   buffer_len Length of the data buffer
 * @return  BOOTLOADER_ERROR_NONE if data is written successfully
 *          BOOTLOADER_FLASH_WRITE_OUT_OF_BOUNDS if address is out of bounds
 *          BOOTLOADER_FLASH_WRITE_NOT_ALIGNED if data is not aligned
 */
BootloaderError boot_flash_write(uintptr_t address, uint8_t *buffer, size_t buffer_len);

/**
 * @brief   Erase some number of flash pages
 * @param   start_page Initial page number to erase
 * @param   num_pages Number of pages to erase
 * @return  BOOTLOADER_ERROR_NONE if data is erased successfully
 *          BOOTLOADER_FLASH_ERR if page is out of bounds OR erasing failed
 */
BootloaderError boot_flash_erase(uint8_t start_page, uint8_t num_pages);

/**
 * @brief   Read from flash memory
 * @param   address Base memory address to read from
 * @param   buffer Pointer to the data buffer
 * @param   buffer_len Length of the data buffer
 * @return  BOOTLOADER_ERROR_NONE if data is read successfully
 *          BOOTLOADER_FLASH_READ_FAILED if data read failed
 */
BootloaderError boot_flash_read(uintptr_t address, uint8_t *buffer, size_t buffer_len);

/**
 * @brief   Verify that the flash memory is not erased
 * @details This checks the entire application flash memory to ensure it is not entirely erased
 * @return  BOOTLOADER_ERROR_NONE if the memomry is valid
 *          BOOTLOADER_FLASH_ERR if the memory is corrupt
 */
BootloaderError boot_verify_flash_memory(void);

/** @} */
