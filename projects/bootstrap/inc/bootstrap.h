#pragma once

/************************************************************************************************
 * @file   bootstrap.h
 *
 * @brief  Header file for bootstrap application
 *
 * @date   2025-09-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>
#include <string.h>

/* Inter-component Headers */
#include "midFS.h"

/* Intra-component Headers */

/**
 * @defgroup bootstrap
 * @brief    bootstrap Firmware
 * @{
 */

#define BOOTLOADER_ADDR 0x08008000
#define BOOTLOADER_SIZE 0x00007000  // 64 KB
#define APPLICATION_ADDR 0x08018000
#define CRC_FILE_PATH "crc.txt"
#define CRC_SIZE 8
#define CRC32_POLY 0xEDB88320UL  // Reversed 0x04C11DB7

extern uint32_t crc32_table[256];

typedef void (*EntryPoint)(void);

/**
 * Jumps to a specific address in memory
 *
 */
void jump_to(uint32_t addr);

/**
 * Initializes the crc32 table
 *
 */
void crc32_init(void);

/**
 * Manually computes CRC
 *
 */
void compute_crc32(const uint8_t *data, size_t length, uint32_t *crc_dest);

/**
 * Reads CRC value from file system
 *
 */
FsStatus read_crc32(const char *file_path, size_t length, uint32_t *crc_dest);

/**
 * Main application executed at runtime
 *
 */
void bootstrap_main(void);
/** @} */
