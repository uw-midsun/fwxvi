#include <stdint.h>

#define BLOCKS_PER_GROUP 32
#define BLOCK_SIZE 512
#define FILE_ENTRY_SIZE 40 //sizeof(FileEntry)
#define FOLDER_CAPACITY 512
#define MAX_FILENAME_LENGTH 32

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

typedef struct{
    uint8_t blockBitmap[BLOCKS_PER_GROUP];
    uint8_t dataBlocks[BLOCKS_PER_GROUP][BLOCK_SIZE];
    uint32_t nextBlockGroup; // address of the next block group
}BlockGroup;
    
typedef struct{
    uint32_t magic;
    uint16_t blockSize;
    uint16_t blocksPerGroup;
    uint8_t rootDirectory[FOLDER_CAPACITY];
    uint16_t numBlocks;
    uint32_t nextBlockGroup; //address of the next (first) block group
}SuperBlock;