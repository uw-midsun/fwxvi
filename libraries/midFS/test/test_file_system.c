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
#include "file_system.h"
/* External library Headers */

/* Intra-component Headers */

StatusCode status;

void setup_test(void) {

    // Initialize the Unity test framework
    UNITY_BEGIN();

    // Initialize the file system if needed
<<<<<<< HEAD:libraries/midFS/tests/test_file_system.c
    file_system_init();

=======
    status = fs_init();
    printf("%d", status);
    
>>>>>>> f87b5a8 (bugfixes):libraries/midFS/test/test_file_system.c
    // Additional setup can be done here
    printf("Test setup complete.\n");
    
}

void teardown_test(void) {}




TEST_IN_TASK
void test_superblock_and_first_block_setup(void){
    TEST_ASSERT_NOT_NULL(superblock);
    TEST_ASSERT_EQUAL_HEX32(0xC1D1921D, superblock->magic);
    TEST_ASSERT_EQUAL(BLOCK_SIZE< superBlock->blockSize);
    TEST_ASSERT_EQUAL(BLOCKS_PER_GROUP, superBlock->blocksPerGroup);
    //test offset is correct
    TEST_ASSERT_EQUAL_UINT8(*(fs_memory + sizeof(SuperBlock)), ((uint8_t *)blockGroups)[0]);

    FileEntry *root=&superBlock->rootFOlderMetadata;
    TEST_ASSERT_EQUAL_STRING("/", root->fileName);
    TEST_ASSERT_EQUAL(FILETYPE_FOLDER< root->fileName);
    TEST_ASSERT_EQUAL(1, root->valid);
    TEST_ASSERT_EQUAL_UINT16(0, root->startBlockIndex);
    TEST_ASSERT_EQUAL_UINT32(0, root->size);

    BlockGroup* group0=&blockGroups[0];
    TEST_ASSERT_EQUAL_UINT8(FS_NULL_BLOCK_GROUP, group0->nextBlockGroup);
    //block 0 should be marked, others aren't
    TEST_ASSERT_EQUAL_UINT8(1, group0->blockBitmap[0]);    
    for (int i=1; i<BLOCKS_PER_GROUP; i++){
        TEST_ASSERT_EQUAL_UINT*(0, group0->blockBitmap[i]);
    }
    //all data blocks should be zeroed
    for (int i=0; i<BLOCKS_PER_GROUP; i++){
        for (int j=0; j<BLOCK_SIZE; j++){
            TEST_ASSERT_EQUAL_UINT8(0, group0->dataBlocks[i][j]);
        }
    }
}

TEST_IN_TASK
void test_max_filename_length(void){
    char long_name[MAX_FILENAME_LENGTH];
}

TEST_IN_TASK
void test_multi_block_wrie(void){

}


TEST_IN_TASK
void test_add_empty_file(void){

}
void test_example(void) {
    printf("test example begin\n");

    status = fs_add_file("test.txt", (uint8_t *)"This is a test file.", 21, 0);

    printf("%d", status);

    uint32_t parentBlockLocation;

    status = fs_read_file("/test.txt");
    printf("%d", status);

}