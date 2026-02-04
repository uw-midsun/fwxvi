/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for arena_test
 *
 * @date   2026-02-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "status.h"
#include "delay.h"
#include "arena.h"

/* Intra-component Headers */

#define BUFFER_SIZE 1024
#define BUFFER_ALLIGN_4 4
#define BUFFER_ALLIGN_8 8
#define BUFFER_ALLIGN_16 16

uint8_t buffer[BUFFER_SIZE] = { 0 };

typedef struct {
  int x;
  float y;
} MyStruct;

TASK(arena_api, TASK_STACK_1024) {
  Arena arena_obj;
  StatusCode ret = STATUS_CODE_OK;

  memset(buffer, 0xFF, BUFFER_SIZE);
  ret = arena_init(&arena_obj, buffer, BUFFER_SIZE);

  if (ret != STATUS_CODE_OK) {
    LOG_DEBUG("ERROR arena_init");
  }

  while (true) {
    MyStruct *obj = NULL;

    ret = arena_alloc(&arena_obj, sizeof(MyStruct), _Alignof(MyStruct), 1, (void**)&obj);
    if (ret != STATUS_CODE_OK) {
      LOG_DEBUG("ERROR arena_alloc");
      continue;
    }

    obj->x = 10;
    obj->y = 3.14f;

    ret = arena_reset(&arena_obj);
    if (ret != STATUS_CODE_OK) {
      LOG_DEBUG("ERROR arena_reset");
    }
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

  tasks_init_task(arena_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
