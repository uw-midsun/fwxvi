#include <stdint.h>
#include "midFS_types.h"

SuperBlock superBlock = {
    .magic = 0xC1D1921D,
    .maxFiles = MAX_FILES_PER_GROUP * BLOCKS_PER_GROUP * BLOCK_SIZE,
    .blockSize = BLOCK_SIZE,
    .blocksPerGroup = BLOCKS_PER_GROUP,
    .totalSize = MAX_FILES_PER_GROUP * BLOCKS_PER_GROUP * BLOCK_SIZE,
    .rootIndex = 0,
    .numBlocks = 0
};


void fs_init();
void fs_create_block_group();
void fs_add_file(const char * name, uint32_t size, uint8_t isFolder);
void fs_delete_file(const char *name);
FileEntry fs_read_file(const char * name);
uint8_t *fs_extract_file(uint16_t address);
void fs_write_file(const char * name, uint8_t * content, uint32_t contentSize);