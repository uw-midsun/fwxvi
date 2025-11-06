#pragma once

/************************************************************************************************
 * @file    bootloader_memory_map.h
 *
 * @brief   Bootloader Memory Map
 *
 * @date    2025-10-04
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup bootloader
 * @brief    bootloader Firmware
 * @{
 */

extern uint32_t _flash_start;
extern uint32_t _flash_size;
extern uint32_t _flash_page_size;

extern uint32_t _bootstrap_start;
extern uint32_t _bootstrap_size;

extern uint32_t _bootloader_start;
extern uint32_t _bootloader_size;

extern uint32_t _app_active_start;
extern uint32_t _app_active_size;

extern uint32_t _app_staging_start;
extern uint32_t _app_staging_size;

extern uint32_t _fs_storage_start;
extern uint32_t _fs_storage_size;

extern uint32_t _sram_start;
extern uint32_t _sram_size;

/** @brief  Flash start address as defined in the linkerscripts */
#define FLASH_START_ADDRESS_LINKERSCRIPT ((uint32_t) & _flash_start)

/** @brief  Flash size as defined in the linkerscripts */
#define FLASH_SIZE_LINKERSCRIPT ((size_t) & _flash_size)

/** @brief  Flash page size as defined in the linkerscripts */
#define FLASH_PAGE_SIZE_LINKERSCRIPT ((size_t) & _flash_page_size)

/** @brief  Application Active start address as defined in the linkerscripts */
#define APP_ACTIVE_START_ADDRESS ((uint32_t) & _app_active_start)

/** @brief  Application Active size as defined in the linkerscripts */
#define APP_ACTIVE_SIZE ((size_t) & _app_active_size)

/** @brief  Application Staging start address as defined in the linkerscripts */
#define APP_STAGING_START_ADDRESS ((uint32_t) & _app_staging_start)

/** @brief  Application Staging size as defined in the linkerscripts */
#define APP_STAGING_SIZE ((size_t) & _app_staging_size)

/** @brief  Bootloader start address as defined in the linkerscripts */
#define BOOTLOADER_START_ADDRESS ((uint32_t) & _bootloader_start)

/** @brief  Bootloader size as defined in the linkerscripts */
#define BOOTLOADER_SIZE ((size_t) & _bootloader_size)

/** @brief  Bootstrap start address as defined in the linkerscripts */
#define BOOTSTRAP_START_ADDRESS ((uint32_t) & _bootstrap_start)

/** @brief  Bootstrap size as defined in the linkerscripts */
#define BOOTSTRAP_SIZE ((size_t) & _bootstrap_size)

/** @brief  File System Storage start address as defined in the linkerscripts */
#define FS_STORAGE_START_ADDRESS ((uint32_t) & _fs_storage_start)

/** @brief  File System Storage size as defined in the linkerscripts */
#define FS_STORAGE_SIZE ((size_t) & _fs_storage_size)

/** @brief  SRAM start address as defined in the linkerscripts */
#define SRAM_START_ADDRESS ((uint32_t) & _sram_start)

/** @brief  SRAM size as defined in the linkerscripts */
#define SRAM_SIZE ((size_t) & _sram_size)

/** @} */
