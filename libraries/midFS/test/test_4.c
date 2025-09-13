/************************************************************************************************
 * @file   test_0.c
 *
 * @brief  Test file for file_system - testing adding/reading/writing files of various sizes in the root directory
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

/* Intra-component Headers */

StatusCode status;

void setup_test(void){
    UNITY_BEGIN();
    status=fs_init();
    printf("%d", status);
    printf("Test setup complete \n");
}


void teardown_test(void){}

void test_bs(void){
    
}

void test_fs_read_file_nonexistent(void){
    const char* file_path="/log.txt";
    const char* initial_content="Initial log";
    const char* extra_content="Overwrite";

    StatusCode status=fs_add_file(file_path, (uint8_t*)extra_content, strlen(extra_content), 0);
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);
    status=fs_write_file(file_path, (uint8_t *)extra_content, strlen(extra_content));
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);

    uint8_t *content;

    status=fs_read_file(file_path, content);
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);

}

void test_overwrite_expand_file(void){
    const char *path="./big.txt";
    char large[BLOCK_SIZE*2];
    memset(large, 'A', sizeof(large));
    StatusCode s = fs_add_file(path, (uint8_t *)"Start", 5, 0);
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, 5);
    s=fs_write_file(path, (uint8_t*)large, sizeof(large));
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, s);
}

void test_overwrite_shrink_file(void){
    const char* path="/shrink.txt";
    char data[BLOCK_SIZE*2];
    memset(data, 'Z', sizeof(data));

    StatusCode s=fs_add_file(path, (uint8_t*) data, sizeof(data), 0);
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, s);
    const char* shorter="Tiny";
    s = fs_write_file(path, (uint8_t *)shorter, strlen(shorter));
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, s);

}

void test_multiple_overwrites(void){
    const char* path="/multi.txt";

    StatusCode s=fs_add_file(path, (uint8_t *)"First", 5, 0);
    TEST_ASSERT_EQUAL(STATUS_CODE_OK,s);
    const char* v2="Second";
    s=fs_write_file(path, (uint8_t *)v2, strlen(v2));
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, s);
    const char* v3="This is a very long and wordy block of text.";
    s=fs_write_file(path, (uint8_t *)v3, strlen(v3));
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, s);
}


