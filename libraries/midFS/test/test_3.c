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
    status=fs_read_file("/nonexistent.txt");
    TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);

    status = fs_delete_file("/nonexistent.txt");
    TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);

    const char *data="I dont exist";
    status= fs_write_file("/nonexistent.txt", (uint8_t *)data, strlen(data));
    TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);
}



