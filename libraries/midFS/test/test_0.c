/************************************************************************************************
 * @file   test_0.c
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
    status = fs_add_file("/test.txt", (uint8_t *)"This is a test file.", 21, 0);
    status = fs_add_file("/test2.txt", (uint8_t *)"This is another test file.", 27, 0);
    status = fs_add_file("/testlarge.txt", (uint8_t *)"This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. END", 652, 0);
    status = fs_add_file("/test3.txt", (uint8_t *)"This test file goes after a large test file to see if anything gets messed up", 78, 0);
    
    uint32_t parentBlockLocation;
    
    status = fs_read_file("/test.txt");
    status = fs_read_file("/test2.txt");
    status = fs_read_file("/testlarge.txt");
    status = fs_read_file("/test3.txt");
    
    status = fs_delete_file("/test3.txt");
    status = fs_read_file("/test3.txt");
    
    status = fs_write_file("/test.txt", (uint8_t *)" This is some additional content large enough so that we can no longer write in place. This is some additional content large enough so that we can no longer write in place. This is some additional content large enough so that we can no longer write in place. This is some additional content large enough so that we can no longer write in place. MIDDLE This is some additional content large enough so that we can no longer write in place. This is some additional content large enough so that we can no longer write in place. END", 528);
    status = fs_read_file("/test.txt");

    status = fs_write_file("/testlarge.txt", (uint8_t *)" This is some additional content", 33);

    status = fs_read_file("/testlarge.txt");
    
    status = fs_list("/");
    
    status = fs_delete_file("/test2.txt");
    status = fs_list("/");

}