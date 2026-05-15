#pragma once

/************************************************************************************************
 * @file   bootstrap_memory_map.h
 *
 * @brief  Memory map for bootstrap application
 *
 * @date   2026-05-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup bootstrap
 * @brief    bootstrap Firmware
 * @{
 */

#ifdef MS_PLATFORM_X86

#define FLASH_START_ADDRESS_LINKERSCRIPT ((uint32_t)0x08000000U)
#define FLASH_SIZE_LINKERSCRIPT ((size_t)(256U * 1024U))

#define BOOTSTRAP_START_ADDRESS ((uint32_t)FLASH_START_ADDRESS_LINKERSCRIPT)
#define BOOTSTRAP_SIZE ((size_t)(32U * 1024U))

#define BOOTLOADER_START_ADDRESS ((uint32_t)(FLASH_START_ADDRESS_LINKERSCRIPT + 0x8000U))
#define BOOTLOADER_SIZE ((size_t)(64U * 1024U))

#define APP_ACTIVE_START_ADDRESS ((uint32_t)(FLASH_START_ADDRESS_LINKERSCRIPT + 0x18000U))
#define APP_ACTIVE_SIZE ((size_t)(80U * 1024U))

#define SRAM_START_ADDRESS ((uint32_t)0x20000000U)
#define SRAM_SIZE ((size_t)(64U * 1024U))

#else

extern uint32_t _flash_start;
extern uint32_t _flash_size;

extern uint32_t _bootstrap_start;
extern uint32_t _bootstrap_size;

extern uint32_t _bootloader_start;
extern uint32_t _bootloader_size;

extern uint32_t _app_active_start;
extern uint32_t _app_active_size;

extern uint32_t _sram_start;
extern uint32_t _sram_size;

#define FLASH_START_ADDRESS_LINKERSCRIPT ((uint32_t)&_flash_start)
#define FLASH_SIZE_LINKERSCRIPT ((size_t)&_flash_size)

#define BOOTSTRAP_START_ADDRESS ((uint32_t)&_bootstrap_start)
#define BOOTSTRAP_SIZE ((size_t)&_bootstrap_size)

#define BOOTLOADER_START_ADDRESS ((uint32_t)&_bootloader_start)
#define BOOTLOADER_SIZE ((size_t)&_bootloader_size)

#define APP_ACTIVE_START_ADDRESS ((uint32_t)&_app_active_start)
#define APP_ACTIVE_SIZE ((size_t)&_app_active_size)

#define SRAM_START_ADDRESS ((uint32_t)&_sram_start)
#define SRAM_SIZE ((size_t)&_sram_size)

#endif

/** @} */
