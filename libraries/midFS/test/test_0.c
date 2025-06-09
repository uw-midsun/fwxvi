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
    status = fs_add_file("test.txt", (uint8_t *)"This is a test file.", 21, 0);
    
    uint32_t parentBlockLocation;
    
    status = fs_read_file("/test.txt");

}