/************************************************************************************************
 * @file   test_file_system.c
 *
 * @brief  Test file for file_system
 *
 * @date   2025-06-05
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "test_helpers.h"
#include "unity.h"
#include "midFS.h"
/* External library Headers */

void setup(void){
    fs_init();
}
void tearDown(void){

}
StatusCode status;

void setup_test(void) {

<<<<<<< HEAD
TEST_IN_TASK
void test_init_superBlock(void){

<<<<<<< HEAD:libraries/midFS/tests/test_file_system.c
    
//check superblock exissts
//check magic number, block size, etc
//root block is first in blockbitmap
//check that i can add files
//check folder is properly declared (members r valid)
}

TEST_IN_TASK
void test_add_files(path, content, size, isFolder){
    //add fileto root, check in root dir and contents written correctly
    //add folder and ensure type is right, can add fiel inside
    //add file with max len name, size
    //add file with more than 1 block
    //fill a block group and trigger a new one to be created
}

TEST_INT_TASK
void test_add_empty_file(void){

}

TEST_IN_TASK
void test_add_file_with_maxnamelen(void){


}

TEST_IN_TASK
void test_add_file_exceeding_max_path(void){

}
TEST_IN_TASK
void test_add_too_large_file(void){

}

TEST_IN_TASK
void test_fill_filesystem_to_capacity(void){
   
}

TEST_IN_TASK
void test_delete_nonexistent_file(void){
    StatusCode status=fs_delete_file("");
}

TEST_IN_TASK
void test_write_to_invalid_file(void){

    // Initialize the file system if needed
    file_system_init();


=======
    // Initialize the file system if needed
>>>>>>> dd8b00e (test structure changed)
    status = fs_init();
    printf("%d", status);
    
    // Additional setup can be done here
    printf("Test setup complete.\n");
    
}

TEST_IN_TASK
void test_fs_resolve_invalid_path(void){
    uint32_t block;
    StatusCode status=fs_resolve_path("/nonexistent", &block);
    TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);
}



// TEST_IN_TASK
// void test_superblock_and_first_block_setup(void){
//     TEST_ASSERT_NOT_NULL(superblock);
//     TEST_ASSERT_EQUAL_HEX32(0xC1D1921D, superblock->magic);
//     TEST_ASSERT_EQUAL(BLOCK_SIZE< superBlock->blockSize);
//     TEST_ASSERT_EQUAL(BLOCKS_PER_GROUP, superBlock->blocksPerGroup);
//     //test offset is correct
//     TEST_ASSERT_EQUAL_UINT8(*(fs_memory + sizeof(SuperBlock)), ((uint8_t *)blockGroups)[0]);

//     FileEntry *root=&superBlock->rootFOlderMetadata;
//     TEST_ASSERT_EQUAL_STRING("/", root->fileName);
//     TEST_ASSERT_EQUAL(FILETYPE_FOLDER< root->fileName);
//     TEST_ASSERT_EQUAL(1, root->valid);
//     TEST_ASSERT_EQUAL_UINT16(0, root->startBlockIndex);
//     TEST_ASSERT_EQUAL_UINT32(0, root->size);

//     BlockGroup* group0=&blockGroups[0];
//     TEST_ASSERT_EQUAL_UINT8(FS_NULL_BLOCK_GROUP, group0->nextBlockGroup);
//     //block 0 should be marked, others aren't
//     TEST_ASSERT_EQUAL_UINT8(1, group0->blockBitmap[0]);    
//     for (int i=1; i<BLOCKS_PER_GROUP; i++){
//         TEST_ASSERT_EQUAL_UINT*(0, group0->blockBitmap[i]);
//     }
//     //all data blocks should be zeroed
//     for (int i=0; i<BLOCKS_PER_GROUP; i++){
//         for (int j=0; j<BLOCK_SIZE; j++){
//             TEST_ASSERT_EQUAL_UINT8(0, group0->dataBlocks[i][j]);
//         }
//     }
// }

// TEST_IN_TASK
// void test_max_filename_length(void){
//     char long_name[MAX_FILENAME_LENGTH];
// }

// TEST_IN_TASK
// void test_multi_block_write(void){

// }


// TEST_IN_TASK
// void test_add_empty_file(void){

// }

TEST_IN_TASK
void test_example(void) {
    printf("test example begin\n");

    status = fs_add_file("test.txt", (uint8_t *)"This is a test file.", 21, 0);

    printf("%d", status);

    uint32_t parentBlockLocation;

    status = fs_read_file("/test.txt");
    printf("%d", status);

}