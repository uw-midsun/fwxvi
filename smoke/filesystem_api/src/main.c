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
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "midFS.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */

#define FILESYSTEM_WRITER_MODE 0U
#define FILESYSTEM_READER_MODE 1U

#define FILESYSTEM_MODE FILESYSTEM_READER_MODE

TASK(filesystem_api, TASK_STACK_1024) {
  FsStatus fs_status = FS_STATUS_OK;

#if (FILESYSTEM_MODE == FILESYSTEM_WRITER_MODE)
  LOG_DEBUG("\r\nFile system init\r\n");
  fs_status = fs_init();
  if (fs_status != FS_STATUS_OK) {
    LOG_DEBUG("fs_init() failed with exit code %u\r\n", fs_status);
  }
  LOG_DEBUG("filesystem_api test - writer mode\r\n");
  fs_status = fs_add_file("/crc.txt", (uint8_t *)"CRCPOLY", 8, 0);
  if (fs_status != FS_STATUS_OK) {
    LOG_DEBUG("fs_add_file() failed with exit code %u\r\n", fs_status);
  }

  fs_list("/");
  fs_status = fs_commit();
  if (fs_status != FS_STATUS_OK) {
    LOG_DEBUG("fs_commit() failed with exit code %u\r\n", fs_status);
  } else {
    LOG_DEBUG("fs_commited() successfully\r\n");
  }

#else
  LOG_DEBUG("filesystem_api test - reader mode\r\n");

  fs_status = fs_pull();
  if (fs_status != FS_STATUS_OK) {
    LOG_DEBUG("fs_pull() failed with exit code %u\r\n", fs_status);
  }

  fs_list("/");

  uint8_t crc_poly[8] = { 0 };

  fs_status = fs_read_file("/crc.txt", crc_poly);
  if (fs_status != FS_STATUS_OK) {
    LOG_DEBUG("fs_read_file() failed with exit code %u\r\n", fs_status);
  }

  LOG_DEBUG("Read data: %s\n", crc_poly);

#endif

  while (true) {
  }
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
