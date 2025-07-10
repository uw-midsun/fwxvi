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
  fs_add_file("/test.txt", (uint8_t *)"This is a test file.", 21, 0);
  // fs_add_file("/test2.txt", (uint8_t *)"This is a test file2.", 22, 0);
  // fs_add_file("/testlarge.txt", (uint8_t *)"This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. END", 652, 0);
  fs_add_file("/testlarge.txt", (uint8_t *)"This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple blocks to be stored. This is a test file that is so large that it needs multiple bloEND", 310, 0);

  fs_read_file("/test.txt");
  // fs_read_file("/test2.txt");
  fs_read_file("/testlarge.txt");

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