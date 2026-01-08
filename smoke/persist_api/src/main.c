/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for persist_api
 *
 * @date   2025-09-16
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/**
 * !NOTE: there is a bug where if flash memory is full, all future persist_commit()
 * calls will fail with exit code 10. Can be resolved by running flash_api smoke test
 */

/* Standard library Headers */
#include <stdio.h>

/* Inter-component Headers */
#include "delay.h"
#include "flash.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "persist.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */

#define PERSIST_WRITER_MODE 0U
#define PERSIST_READER_MODE 1U
#define LAST_PAGE (NUM_FLASH_PAGES - 1)

#define PERSIST_MODE PERSIST_READER_MODE

// Test struct must be 4 byte aligned
typedef struct __attribute__((aligned(4))) TestStruct {
  uint16_t x;
  uint16_t y;
  char z;
  uint8_t _pad[3];
} TestStruct;

TestStruct test_struct = { 0U };
PersistStorage storage = { 0U };

TASK(persist_api, TASK_STACK_1024) {
  StatusCode status = STATUS_CODE_OK;

  flash_init();
#if (PERSIST_MODE == PERSIST_WRITER_MODE)
  LOG_DEBUG("Starting persist smoke test - writer mode\r\n");
  delay_ms(10U);
  if (sizeof(test_struct) % FLASH_MEMORY_ALIGNMENT != 0) {
    LOG_DEBUG("Warning: size of test struct: %u is not %u-byte aligned!!\r\n", sizeof(test_struct), FLASH_MEMORY_ALIGNMENT);
    delay_ms(10U);
  }

  status = persist_init(&storage, LAST_PAGE, &test_struct, sizeof(test_struct), true);

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("persist_init() failed with exit code %u\r\n", status);
    delay_ms(10U);
  }

  test_struct.x = 32;
  test_struct.y = 2;
  test_struct.z = 't';

  status = persist_commit(&storage);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("persist_commit() failed with exit code %u\r\n", status);
    delay_ms(10U);
    LOG_DEBUG("persist memory may be corrupted... run flash_api smoke test to clear memory\r\n");
    delay_ms(10U);
  }

  LOG_DEBUG("Writer commited.\r\n");
  delay_ms(10U);
  LOG_DEBUG("Wrote test struct X: %u, Y: %u, Z: %c\r\n", test_struct.x, test_struct.y, test_struct.z);
  delay_ms(10U);

#else
  LOG_DEBUG("Starting persist smoke test - reader mode\r\n");
  delay_ms(10U);
  status = persist_init(&storage, LAST_PAGE, &test_struct, sizeof(test_struct), false);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("persist_init() failed with exit code %u\r\n", status);
    delay_ms(10U);
  }
  LOG_DEBUG("Read test struct X: %u, Y: %u, Z: %c\r\n", test_struct.x, test_struct.y, test_struct.z);

#endif

  while (true) {
  }
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
