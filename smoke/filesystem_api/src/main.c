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

// TASK(filesystem_api, TASK_STACK_1024) {
//   fs_init();
//   LOG_DEBUG("File system init\n");
//   fs_add_file("/crc.txt", (uint8_t *)"CRCPOLY", 8, 0);
//   fs_list("/");
//   fs_commit();

//   fs_pull();
//   fs_list("/");

//   uint8_t crc_poly[8] = {0};

//   fs_read_file("/crc.txt", crc_poly);

//   LOG_DEBUG("%s\n", crc_poly);

//   while (true) {}
// }

int main() {
  mcu_init();
  fs_init();
  // LOG_DEBUG("File system init\n");
  fs_add_file("/crc.txt", (uint8_t *)"CRCPOLY", 8, 0);
  fs_list("/");
  fs_commit();

  fs_pull();
  fs_list("/");

  uint8_t crc_poly[8] = {0};

  fs_read_file("/crc.txt", crc_poly);

  printf("%s\n", crc_poly);
  // tasks_init();
  // log_init();
  
  // tasks_init_task(filesystem_api, TASK_PRIORITY(3), NULL);
  
  // tasks_start();
  
  // LOG_DEBUG("exiting main?");
  
  while (true) {
    printf("FS_init");
  }

  return 0;
}
