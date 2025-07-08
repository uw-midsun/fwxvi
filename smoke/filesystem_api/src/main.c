/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for filesystem_api
 *
 * @date   2025-06-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "status.h"
#include "delay.h"
#include "midFS.h"

/* Intra-component Headers */

TASK(filesystem_api, TASK_STACK_1024) {
  fs_init();
  // fs_add_file("/test.txt", (uint8_t *)"This is a test file.", 21, 0);
  // fs_add_file("/test2.txt", (uint8_t *)"This is another test file.", 27, 0);
  // fs_add_file("/testlarge.txt", (uint8_t *)"This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. END", 652, 0);
  // fs_add_file("/test3.txt", (uint8_t *)"This test file goes after a large test file to see if anything gets messed up", 78, 0);
  
  // fs_read_file("/test.txt");
  // fs_read_file("/test2.txt");
  // fs_read_file("/testlarge.txt");
  // fs_read_file("/test3.txt");
    
  // fs_delete_file("/test3.txt");

  // fs_write_file("/test.txt", (uint8_t *)" This is some additional content large enough so that we can no longer write in place. This is some additional content large enough so that we can no longer write in place. This is some additional content large enough so that we can no longer write in place. This is some additional content large enough so that we can no longer write in place. MIDDLE This is some additional content large enough so that we can no longer write in place. This is some additional content large enough so that we can no longer write in place. END", 528);
  // fs_read_file("/test.txt");

  // fs_write_file("/testlarge.txt", (uint8_t *)" This is some additional content", 33);

  // fs_read_file("/testlarge.txt");
  // fs_list("/");
    
  // fs_delete_file("/test2.txt");
  // fs_list("/");
  fs_add_file("root", NULL, 0, 1);
  fs_add_file("root/test0.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root/test1.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root/test2.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root/test3.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root/test4.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root/test5.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root/test6.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root/test7.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root/test8.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root/test9.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root/test10.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root/test11.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root/test12.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root2", NULL, 0, 1);
  fs_add_file("root2/test0.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root2/test1.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root2/test2.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root2/test3.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root2/test4.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root2/test5.txt", (uint8_t *)"This is a test file.", 21, 0);
  fs_add_file("root2/test6.txt", (uint8_t *)"This is a test file.", 21, 0);

  fs_list("/root");
  fs_list("/root2");


  while (true) {}
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(filesystem_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}