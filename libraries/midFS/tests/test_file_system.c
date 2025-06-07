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

void setup_test(void) {

    // Initialize the Unity test framework
    UNITY_BEGIN();

    // Initialize the file system if needed
    file_system_init();

    // Additional setup can be done here
    printf("Test setup complete.\n");
    
}

void teardown_test(void) {}

void test_example(void) {
    printf("test example begin\n");
    fs_add_file("test.txt", "This is a test file.", 21, 0);

    uint32_t parentBlockLocation;

    fs_read_file("/test.txt");

}

int main(void) {
    UNITY_BEGIN();

    setup_test();

    RUN_TEST(test_example);

    teardown_test();

    return UNITY_END();
}