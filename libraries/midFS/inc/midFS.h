#include <stdint.h>
#include "midFS_types.h"
#include "status.h"

#define FS_NULL_BLOCK_GROUP 0xFFFFFFFF
#define FS_INVALID_BLOCK 0xFFFFFFF

/**
 * Initializes the file system, must be called
 * 
 */
StatusCode fs_init();

/**
 * Helper function that creates a block group at the next available index
 * 
 * @return  STATUS_CODE_INCOMPLETE if there is no more space
 */
StatusCode fs_create_block_group(uint32_t *index);

/**
 * Helper function that reads the contents of a block group at index blockIndex into local parameter dest
 * 
 * @return  STATUS_CODE_OUT_OF_RANGE if blockIndex is out of range
 */
StatusCode fs_read_block_group(uint32_t blockIndex, BlockGroup *dest);

/**
 * Helper function to write the contents of local block group src to block group at index blockIndex
 * 
 * @return  STATUS_CODE_OUT_OF_RANGE if blockIndex is out of range
 */
StatusCode fs_write_block_group(uint32_t blockIndex, BlockGroup *src);

/**
 * Add file with given path, content, size
 * 
 * @return  STATUS_CODE_INVALID_ARGS if path is invalid
 */
StatusCode fs_add_file(const char * path, uint8_t* content, uint32_t size, uint8_t isFolder);

/**
 * Delete file with given path name
 * 
 * @return  STATUS_CODE_INVALID_ARGS if path is invalid
 */
StatusCode fs_delete_file(const char* path);

/**
 * Prints out the contents of the file given the path
 */
StatusCode fs_read_file(const char *path);

/**
 * Writes content to a file with given path, can write in place or moves file to new location in memory if needed
 * 
 * @return  STATUS_CODE_INVALID_ARGS if path is invalid
 *          STATUS_CODE_INCOMPLETE if there is no more space
 */
StatusCode fs_write_file(const char * path, uint8_t * content, uint32_t contentSize);

/**
 * Lists all files in a given path, the path must lead to a directory
 * 
 * @return  STATUS_CODE_INVALID_ARGS if path is invalid
 */
StatusCode fs_list(const char * path);

/**
 * Given a file path, store the path name in folderPath and the file name in fileName
 * 
 */
StatusCode fs_split_path(char *path, char *folderPath, char *fileName);

/**
 * Given the path to a folder, return the global index of the block storing the folder content
 * 
 */
StatusCode fs_resolve_path(const char *folderPath, uint32_t* path);