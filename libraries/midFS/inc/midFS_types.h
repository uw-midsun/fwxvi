#pragma once

/************************************************************************************************
 * @file   midFS_types.c
 *
 * @brief  Header file for various types used in file system library
 *
 * @date   2025-09-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup midFS_types
 * @brief    midFS_types Firmware
 * @{
 */

#if defined(__arm__) || defined(__aarch64__)
#define ARCH_ARM 1
#endif

#if defined(__i386__) || defined(__x86_64__)
#define ARCH_X86 1
#endif

#ifdef ARCH_X86
#define BLOCKS_PER_GROUP 8
#define BLOCK_SIZE 512
#define NUM_BLOCK_GROUPS 16
#endif

#ifdef ARCH_ARM
/**
 * Persist library supports max page size of 800U
 * 4 * 90 * 2 = 720
 * 720 + sizeof(SuperBlock) = 780
 *
 * BLOCK_SIZE must be greater than 88 since FileEntry = 44 bits,
 * and blocks must be able to store at least 2 FileEntries
 */
#define BLOCKS_PER_GROUP 4
#define BLOCK_SIZE 90
#define NUM_BLOCK_GROUPS 2
#endif

#define FS_TOTAL_SIZE (BLOCKS_PER_GROUP * BLOCK_SIZE * NUM_BLOCK_GROUPS) + sizeof(SuperBlock)
#define FILE_ENTRY_SIZE 40
#define FOLDER_CAPACITY 512
#define MAX_FILENAME_LENGTH 8
#define MAX_PATH_LENGTH 128

typedef enum {
  FILETYPE_FILE = 0,
  FILETYPE_FOLDER = 1,
} FileType;

typedef struct {
  uint32_t size;
  uint16_t startBlockIndex;
  uint8_t valid;  // 1 -> in use, 0 -> not in use
  FileType type;  // if type == folder, startBlockIndex is the index of an array of files
  char fileName[MAX_FILENAME_LENGTH];
} FileEntry;  // size: 44 bits

typedef struct {
  uint32_t nextBlockGroup;  // address of the next block group
  uint8_t blockBitmap[BLOCKS_PER_GROUP];
  uint8_t dataBlocks[BLOCKS_PER_GROUP][BLOCK_SIZE];
} BlockGroup;

typedef struct {
  uint32_t magic;
  uint32_t nextBlockGroup;  // address of the next (first) block group
  uint16_t blockSize;
  uint16_t blocksPerGroup;
  uint16_t numBlocks;
  FileEntry rootFolderMetadata;
} SuperBlock;  // size: 54 bits
/** @} */
