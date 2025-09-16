/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for persist_api
 *
 * @date   2025-09-16
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdio.h>

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "status.h"
#include "delay.h"
#include "persist.h"
#include "flash.h"

/* Intra-component Headers */

#define PERSIST_WRITER_MODE   0U
#define PERSIST_READER_MODE   1U
#define LAST_PAGE (NUM_FLASH_PAGES - 1)

#define PERSIST_MODE          PERSIST_WRITER_MODE

typedef struct TestStruct {
  uint32_t x;
  float y;
  char z;
} TestStruct;

TestStruct test_struct = { 0U };
PersistStorage storage = { 0U };

TASK(persist_api, TASK_STACK_1024) {
#if (PERSIST_MODE == PERSIST_WRITER_MODE)
persist_init(&storage, LAST_PAGE, &test_struct, sizeof(test_struct), true);

test_struct.x = 32;
test_struct.y = 3.2;
test_struct.z = 't';

persist_commit(&storage);

#else
persist_init(&storage, LAST_PAGE, &test_struct, sizeof(test_struct), false);
LOG_DEBUG("Test struct X: %u, Y: %f, Z: %c", test_struct.x, test_struct.y, test_struct.z);

#endif

  while (true) {}
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main() {
#endif
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(persist_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}