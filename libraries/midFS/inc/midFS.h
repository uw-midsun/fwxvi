#include <stdint.h>
#include "midFS_types.h"

#define FS_NULL_BLOCK_GROUP 0xFFFFFFFF
#define FS_INVALID_BLOCK 0xFFFFFFF

void fs_init();
uint32_t fs_create_block_group();

void fs_read_block_group(uint32_t blockIndex, BlockGroup *dest);
void fs_write_block_group(uint32_t blockIndex, BlockGroup *src);

void fs_add_file(const char * name, uint8_t* content, uint32_t size, uint8_t isFolder);
void fs_add_folder(const char *name);
void fs_delete_file(const char *name);
uint32_t fs_read_file(const char * name);
uint8_t *fs_extract_file(uint16_t address);
void fs_write_file(const char * name, uint8_t * content, uint32_t contentSize);

void fs_split_path(char *path, char *folderPath, char *fileName);
uint32_t fs_resolve_path(const char *folderPath);