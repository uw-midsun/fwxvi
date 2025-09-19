/************************************************************************************************
 * @file   test_1.c
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
#include "midFS.h"
#include "test_helpers.h"
#include "unity.h"
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
  uint8_t *content;

  status = fs_add_file("test.txt", (uint8_t *)"This is a test file.", 21, 0);
  status = fs_add_file("test2.txt", (uint8_t *)"This is another test file.", 27, 0);

  status = fs_add_file("root", NULL, 0, 1);
  status = fs_add_file("root2", NULL, 0, 1);

  fs_add_file("root/rootTest.txt", (uint8_t *)"This is nested a test file.", 28, 0);
  fs_add_file("root/rootTest2.txt", (uint8_t *)"Hi.", 4, 0);
  fs_add_file("root2/root2Test.txt", (uint8_t *)"This is another nested a test file.", 36, 0);
  fs_read_file("/test.txt", content);
  status = fs_read_file("/root/rootTest.txt", content);
  fs_list("/");
  fs_list("root");
  fs_list("root2");

  fs_read_file("/root2/root2Test.txt", content);
  fs_read_file("/root/rootTest.txt", content);
  fs_read_file("/root/rootTest2.txt", content);

  fs_add_file("root/nestedRoot", NULL, 0, 1);
  fs_add_file("root/nestedRoot/nestedRootTest.txt", (uint8_t *)"This is a double nested test file.", 35, 0);

  fs_read_file("/root/nestedRoot/nestedRootTest.txt", content);
  fs_list("root/nestedRoot");
  fs_list("root");

  uint8_t doesFileExist;

  fs_does_file_exist("root2/root2Test.txt", &doesFileExist);
  printf("Does file exist: %d\n", doesFileExist);
  fs_add_file("root2/root2Test.txt", (uint8_t *)"This is another nested a test file.", 36, 0);
}
