#include <stdint.h>

#define BLOCKS_PER_GROUP 4
#define BLOCK_SIZE 256
#define NUM_BLOCK_GROUPS 4
// #define FS_TOTAL_SIZE (BLOCKS_PER_GROUP*BLOCK_SIZE*NUM_BLOCK_GROUPS) + sizeof(SuperBlock)
#define FS_TOTAL_SIZE (sizeof(BlockGroup) * NUM_BLOCK_GROUPS) + sizeof(SuperBlock)
#define FILE_ENTRY_SIZE sizeof(FileEntry)
#define MAX_FILENAME_LENGTH 32
#define MAX_PATH_LENGTH 128

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
}FileEntry;

//size: 40 bytes

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
}SuperBlock;

//size: 54 bytes