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

void setup(void){
    fs_init();
}
void tearDown(void){

}

TEST_IN_TASK
void test_init_superBlock(void){

    
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
    
}

TEST_IN_TASK
void test_fs_resolve_invalid_path(void){
    uint32_t block;
    StatusCode status=fs_resolve_path("/nonexistent", &block);
    TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);
}


void test_example(void) {
    fs_add_file("test.txt", "This is a test file.");

    uint32_t parentBlockLocation;

    fs_read_file("/test.txt");

}