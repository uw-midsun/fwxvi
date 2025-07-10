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

/* Intra-component Headers */
#include "fota_packet.h"

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

#ifdef MS_PLATFORM_X86

/** @brief  Flash start address as defined in the linkerscripts */
#define FLASH_START_ADDRESS_LINKERSCRIPT ((uint32_t)0x08000000U)

/** @brief  Flash size as defined in the linkerscripts (e.g., 1MB) */
#define FLASH_SIZE_LINKERSCRIPT ((size_t)(1024 * 1024U))

/** @brief  Flash page size as defined in the linkerscripts (e.g., 2KB per page) */
#define FLASH_PAGE_SIZE_LINKERSCRIPT ((size_t)(2 * 1024U))

/** @brief  Application start address as defined in the linkerscripts */
#define APPLICATION_START_ADDRESS ((uint32_t)(FLASH_START_ADDRESS_LINKERSCRIPT + 0x10000U))

/** @brief  Application size (e.g., 512KB) */
#define APPLICATION_SIZE ((size_t)(512 * 1024U))

/** @brief  Bootloader start address as defined in the linkerscripts */
#define BOOTLOADER_START_ADDRESS (FLASH_START_ADDRESS_LINKERSCRIPT)

/** @brief  Bootloader size (e.g., 64KB) */
#define BOOTLOADER_SIZE ((size_t)(64 * 1024U))

/** @brief  SRAM start address as defined in the linkerscripts */
#define SRAM_START_ADDRESS ((uint32_t)0x20000000U)

/** @brief  SRAM size as defined in the linkerscripts (e.g., 128KB) */
#define SRAM_SIZE ((size_t)(128 * 1024U))

#else

extern uint32_t _flash_start;
extern uint32_t _flash_size;
extern uint32_t _flash_page_size;

extern uint32_t _accumulator_memory;
extern uint32_t _accumulator_memory_size;

extern uint32_t _application_start;
extern uint32_t _application_size;

extern uint32_t _bootloader_start;
extern uint32_t _bootloader_size;

extern uint32_t _bios_start;
extern uint32_t _bios_size;

extern uint32_t _sram_start;
extern uint32_t _sram_size;

/** @brief  Flash start address as defined in the linkerscripts */
#define FLASH_START_ADDRESS_LINKERSCRIPT ((uint32_t) & _flash_start)

/** @brief  Flash size as defined in the linkerscripts */
#define FLASH_SIZE_LINKERSCRIPT ((size_t) & _flash_size)

/** @brief  Flash page size as defined in the linkerscripts */
#define FLASH_PAGE_SIZE_LINKERSCRIPT ((size_t) & _flash_page_size)

/** @brief  Application start address as defined in the linkerscripts */
#define APPLICATION_START_ADDRESS ((uint32_t) & _application_start)

/** @brief  Application size as defined in the linkerscripts */
#define APPLICATION_SIZE ((size_t) & _application_size)

/** @brief  Bootloader start address as defined in the linkerscripts */
#define BOOTLOADER_START_ADDRESS ((uint32_t) & _bootloader_start)

/** @brief  Bootloader size as defined in the linkerscripts */
#define BOOTLOADER_SIZE ((size_t) & _bootloader_size)

/** @brief  SRAM start address as defined in the linkerscripts */
#define SRAM_START_ADDRESS ((uint32_t) & _sram_start)

/** @brief  SRAM size as defined in the linkerscripts */
#define SRAM_SIZE ((size_t) & _sram_size)

#endif

/** @} */
