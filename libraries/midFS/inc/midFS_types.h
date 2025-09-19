#pragma once

/************************************************************************************************
 * @file   midFS_types.c
 *
 * @brief  Header file for various types used in file system library
 *
 * @date   2025-09-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

#include <stdint.h>

#define BLOCKS_PER_GROUP 64
#define BLOCK_SIZE 512
#define NUM_BLOCK_GROUPS 64
#define FS_TOTAL_SIZE (BLOCKS_PER_GROUP*BLOCK_SIZE*NUM_BLOCK_GROUPS) + sizeof(SuperBlock)
#define FILE_ENTRY_SIZE 40
#define FOLDER_CAPACITY 512
#define MAX_FILENAME_LENGTH 32
#define MAX_PATH_LENGTH 128
#define MAX_FILE_CONTENT_POST_WRITE 2056

typedef enum{
    FILETYPE_FILE = 0,
    FILETYPE_FOLDER = 1
}FileType;

typedef struct{
    char fileName[MAX_FILENAME_LENGTH];
    uint32_t size;
    uint16_t startBlockIndex;
    uint8_t valid; //1 -> in use, 0 -> not in use
    FileType type; //if type == folder, startBlockIndex is the index of an array of files
}FileEntry; //size: 40 bytes

typedef struct{
    uint8_t blockBitmap[BLOCKS_PER_GROUP];
    uint8_t dataBlocks[BLOCKS_PER_GROUP][BLOCK_SIZE];
    uint32_t nextBlockGroup; // address of the next block group
}BlockGroup;

    
typedef struct{
    uint32_t magic;
    uint16_t blockSize;
    uint16_t blocksPerGroup;
    uint16_t numBlocks;
    uint32_t nextBlockGroup; //address of the next (first) block group
    FileEntry rootFolderMetadata;
}SuperBlock; //size: 54 bytes

typedef enum{
    FS_STATUS_INCOMPLETE = -5,
    FS_STATUS_PATH_NOT_FOUND = -4,
    FS_STATUS_OUT_OF_RANGE = -3,
    FS_STATUS_OUT_OF_SPACE = -2,
    FS_STATUS_INVALID_ARGS = -1,
    FS_STATUS_OK = 0 
}FsStatus;