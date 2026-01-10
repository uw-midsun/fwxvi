#pragma once

/************************************************************************************************
 * @file   midFS.h
 *
 * @brief  Header file for file system library
 *
 * @date   2025-09-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(__arm__) || defined(__aarch64__)
#define ARCH_ARM 1
#endif

#if defined(__i386__) || defined(__x86_64__)
#define ARCH_X86 1
#endif

/* Inter-component Headers */
#ifdef ARCH_ARM
#include "midFS_hal.h"
#endif
#include "midFS_types.h"

/* Intra-component Headers */
#include "status.h"

/**
 * @defgroup midFS
 * @brief    midFS Firmware
 * @{
 */

#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#define FS_NULL_BLOCK_GROUP 0xFFFFFFFF
#define FS_INVALID_BLOCK 0xFFFFFFF
#define FS_NULL_FILE 0xFFFFFFFF
// #define FS_HAL_ADDRESS 0x8000000 + 0x10000 //8 million + 64KB to hex
#define FS_HAL_ADDRESS 0x0803C000

extern SuperBlock *superBlock;
extern BlockGroup *blockGroups;

// max size: 206KB
// 512B per block, we can have 412 blocks

/**
 * Initializes the file system, must be called
 * Wipes the memory
 * Initializes the first block group and root folder
 *
 * @return FS_STATUS_OK
 */
StatusCode fs_init();

/**
 * Copies FS memory from HAL
 *
 */
StatusCode fs_pull();

/**
 * Writes local copy of fs_memory back to actual one using HAL_write
 *
 */
StatusCode fs_commit();

/**
 * Helper function that creates a block group at the next available index
 *
 * @return  FS_STATUS_OUT_OF_SPACE if there is no more space
 */
StatusCode fs_create_block_group(uint32_t *index);

/**
 * Helper function that reads the contents of a block group at index blockIndex into local parameter dest
 *
 * @return  FS_STATUS_OUT_OF_RANGE if blockIndex is out of range
 */
StatusCode fs_read_block_group(uint32_t blockIndex, BlockGroup *dest);

/**
 * Helper function to write the contents of local block group src to block group at index blockIndex
 *
 * @return  FS_STATUS_OUT_OF_RANGE if blockIndex is out of range
 */
StatusCode fs_write_block_group(uint32_t blockIndex, BlockGroup *src);

/**
 * Add file with given path, content, size
 *
 * @return  FS_STATUS_INVALID_ARGS if path is invalid
 *          FS_STATUS_PATH_NOT_FOUND if we cannot resolve path
 */
StatusCode fs_add_file(const char *path, uint8_t *content, uint32_t size, uint8_t isFolder);

/**
 * Delete file with given path name
 *
 * @return  FS_STATUS_INVALID_ARGS if path is invalid
 *          FS_STATUS_PATH_NOT_FOUND if we cannot resolve path
 *  */
StatusCode fs_delete_file(const char *path);

/**
 * Prints out the contents of the file given the path
 *
 * @return FS_STATUS_INVALID_ARGS if we cannot find file
 *          FS_STATUS_PATH_NOT_FOUND if we cannot resolve path
 */
StatusCode fs_read_file(const char *path, uint8_t *content);

/**
 * Writes content to a file with given path, can write in place or moves file to new location in memory if needed
 *
 * @return  FS_STATUS_INVALID_ARGS if path is invalid
 *          FS_STATUS_OUT_OF_SPACE if there is no more space
 */
StatusCode fs_write_file(const char *path, uint8_t *content, uint32_t contentSize);

/**
 * Lists all files in a given path, the path must lead to a directory
 *
 * @return  FS_STATUS_INVALID_ARGS if path is invalid
 */
StatusCode fs_list(const char *path);

/**
 * Given a file path, store the path name in folderPath and the file name in fileName
 *
 * @return  FS_STATUS_OK
 */
StatusCode fs_split_path(char *path, char *folderPath, char *fileName);

/**
 * Given the path to a folder, return the global index of the block storing the folder content
 *
 * @return  FS_STATUS_PATH_NOT_FOUND if path cannot be found
 */
StatusCode fs_resolve_path(const char *folderPath, uint32_t *path);

/**
 * A function that finds contiguous memory of size blocksNeeded and writes it to incomingBlockAddress
 *
 * @return  FS_STATUS_OUT_OF_SPACE if there is no more space
 */
StatusCode fs_locate_memory(const uint32_t blocksNeeded, uint32_t *incomingBlockAddress);

/**
 * A function that determines if a file with specified path already exists and writes the result to doesFileExist
 *
 * @return  FS_STATUS_PATH_NOT_FOUND if path cannot be found
 */
StatusCode fs_does_file_exist(const char *path, uint8_t *doesFileExist);

#endif  // FILE_SYSTEM_H
/** @} */
