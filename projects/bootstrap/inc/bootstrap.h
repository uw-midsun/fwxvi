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
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */
#include "midFS.h"
#include "status.h"

/* Intra-component Headers */
#include "bootstrap_memory_map.h"

/**
 * @defgroup bootstrap
 * @brief    bootstrap Firmware
 * @{
 */

#define CRC_FILE_PATH "/crc.txt"
#define CRC32_HEX_STRING_SIZE 8U
#define CRC32_POLY 0xEDB88320UL  // Reversed 0x04C11DB7

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
 * @brief Manually computes CRC
 * @param data The data we want to compute the crc for
 * @param length The length of the data in bytes
 * @param crc_dest The variable we want to store the CRC computation result
 * @return void (status codes might be better TODO)
 */
void compute_crc32(const uint8_t *data, size_t length, uint32_t *crc_dest);

/**
 * @brief Parses an 8-character ASCII hex CRC32 value.
 * @param crc_bytes Bytes containing the ASCII hex CRC32 value.
 * @param length Length of crc_bytes. Must be CRC32_HEX_STRING_SIZE.
 * @param crc_dest Destination for the parsed CRC32 value.
 * @return STATUS_CODE_OK if parsed successfully.
 */
StatusCode parse_crc32_hex(const uint8_t *crc_bytes, size_t length, uint32_t *crc_dest);

/**
 * @brief  Reads an ASCII hex CRC32 value from file system.
 * @return STATUS_CODE_OK if the file was read and parsed successfully.
 */
StatusCode read_crc32(const char *file_path, size_t length, uint32_t *crc_dest);

/**
 * @brief Main application executed at runtime (No clue why it was done like this)
 */
void bootstrap_main(void);
/** @} */
