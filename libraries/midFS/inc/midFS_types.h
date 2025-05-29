#include <stdint.h>

#define MAX_FILES_PER_GROUP 128
#define BLOCKS_PER_GROUP 32
#define BLOCK_SIZE 32

typedef enum{
    FILETYPE_FILE = 0,
    FILETYPE_FOLDER = 1
}FileType;

typedef struct{
    char fileName[32];
    uint32_t size;
    uint16_t startBlockIndex;
    uint8_t valid; //1 -> in use, 0 -> not in use
    FileType type; //if type == folder, startBlockIndex is the index of an array of files
}FileEntry;

typedef struct{
    FileEntry entries [MAX_FILES_PER_GROUP];
    uint8_t blockBitmap[BLOCKS_PER_GROUP];
    uint8_t dataBlocks[BLOCKS_PER_GROUP][BLOCK_SIZE];
}BlockGroup;
    
typedef struct{
    uint32_t magic;
    uint16_t maxFiles;
    uint16_t blockSize;
    uint16_t blocksPerGroup;
    uint32_t totalSize;
    uint16_t rootIndex;
}SuperBlock;
