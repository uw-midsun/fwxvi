#pragma once

/************************************************************************************************
 * @file    fota_memory_map.h
 *
 * @brief   Fota Memory Map Header File
 *
 * @date    2025-04-30
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "fota_packet.h"

/* Intra-component Headers */

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

#ifdef MS_PLATFORM_X86

/** @brief  Flash start address as defined in the linkerscripts */
#define FLASH_START_ADDRESS_LINKERSCRIPT ((uint32_t)0x08000000U)

/** @brief  Flash size as defined in the linkerscripts (e.g., 256KB) */
#define FLASH_SIZE_LINKERSCRIPT ((size_t)(256U * 1024U))

/** @brief  Flash page size as defined in the linkerscripts (e.g., 2KB per page) */
#define FLASH_PAGE_SIZE_LINKERSCRIPT ((size_t)(2U * 1024U))

/** @brief  Application Active start address as defined in the linkerscripts */
#define APP_ACTIVE_START_ADDRESS ((uint32_t)(FLASH_START_ADDRESS_LINKERSCRIPT + 0x18000U))

/** @brief  Application Active size (e.g., 80KB) */
#define APP_ACTIVE_SIZE ((size_t)(80U * 1024U))

/** @brief  Application Staging start address as defined in the linkerscripts */
#define APP_STAGING_START_ADDRESS ((uint32_t)(FLASH_START_ADDRESS_LINKERSCRIPT + 0x2A000U))

/** @brief  Application Staging size (e.g., 80KB) */
#define APP_STAGING_SIZE ((size_t)(80U * 1024U))

/** @brief  Bootloader start address as defined in the linkerscripts */
#define BOOTLOADER_START_ADDRESS ((uint32_t)(FLASH_START_ADDRESS_LINKERSCRIPT + 0x8000U))

/** @brief  Bootloader size (e.g., 64KB) */
#define BOOTLOADER_SIZE ((size_t)(64U * 1024U))

/** @brief  Bootstrap start address as defined in the linkerscripts */
#define BOOTSTRAP_START_ADDRESS ((uint32_t)(FLASH_START_ADDRESS_LINKERSCRIPT))

/** @brief  Bootstrap size (e.g., 32KB) */
#define BOOTSTRAP_SIZE ((size_t)(32U * 1024U))

/** @brief  File System Storage start address as defined in the linkerscripts */
#define FS_STORAGE_START_ADDRESS ((uint32_t)(FLASH_START_ADDRESS_LINKERSCRIPT + 0x3C000U))

/** @brief  File System Storage size (e.g., 4KB) */
#define FS_STORAGE_SIZE ((size_t)(4U * 1024U))

/** @brief  SRAM start address as defined in the linkerscripts */
#define SRAM_START_ADDRESS ((uint32_t)0x20000000U)

/** @brief  SRAM size as defined in the linkerscripts (e.g., 64KB) */
#define SRAM_SIZE ((size_t)(64U * 1024U))

#else

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

#endif

/** @} */
