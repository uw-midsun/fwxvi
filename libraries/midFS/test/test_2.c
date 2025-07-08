/************************************************************************************************
 * @file   test_2.c
 *
 * @brief  Test file for file_system - testing nested folders
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

void setup_test(void) {

    // Initialize the Unity test framework
    UNITY_BEGIN();

    // Initialize the file system if needed
    status = fs_init();
    printf("%d", status);
    
    // Additional setup can be done here
    printf("Test setup complete.\n");
    
}

void teardown_test(void) {}

void test_example(void) {
    printf("test 2\n\r");
    status = fs_add_file("root", NULL, 0, 1);
    status = fs_add_file("root/test0.txt", (uint8_t *)"This is a test file.", 21, 0);
    status = fs_add_file("root/test1.txt", (uint8_t *)"This is a test file.", 21, 0);
    status = fs_add_file("root/test2.txt", (uint8_t *)"This is a test file.", 21, 0);
    status = fs_add_file("root/test3.txt", (uint8_t *)"This is a test file.", 21, 0);
    status = fs_add_file("root/test4.txt", (uint8_t *)"This is a test file.", 21, 0);
    // status = fs_add_file("root/test5.txt", (uint8_t *)"This is a test file.", 21, 0);
    // status = fs_add_file("root/test6.txt", (uint8_t *)"This is a test file.", 21, 0);
    // status = fs_add_file("root/test7.txt", (uint8_t *)"This is a test file.", 21, 0);
    // status = fs_add_file("root/test8.txt", (uint8_t *)"This is a test file.", 21, 0);
    // status = fs_add_file("root/test9.txt", (uint8_t *)"This is a test file.", 21, 0);
    // status = fs_add_file("root/test10.txt", (uint8_t *)"This is a test file.", 21, 0);
    // status = fs_add_file("root/test11.txt", (uint8_t *)"This is a test file.", 21, 0);
    // status = fs_add_file("root/test12.txt", (uint8_t *)"This is a test file.", 21, 0);
    
    // status = fs_add_file("root2", NULL, 0, 1);
    // status = fs_add_file("root2/test0.txt", (uint8_t *)"This is a test file.", 21, 0);
    // status = fs_add_file("root2/test1.txt", (uint8_t *)"This is a test file.", 21, 0);
    // status = fs_add_file("root2/test2.txt", (uint8_t *)"This is a test file.", 21, 0);
    // status = fs_add_file("root2/test3.txt", (uint8_t *)"This is a test file.", 21, 0);
    // status = fs_add_file("root2/test4.txt", (uint8_t *)"This is a test file.", 21, 0);
    // status = fs_add_file("root2/test5.txt", (uint8_t *)"This is a test file.", 21, 0);
    // status = fs_add_file("root2/test6.txt", (uint8_t *)"This is a test file.", 21, 0);

    fs_list("/root");
    // fs_list("/root2");

}