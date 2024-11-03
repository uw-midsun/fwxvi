/************************************************************************************************
 * main.c
 *
 * Main file for RTOS Sample.
 *
 * Created: 2024-10-27
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <inttypes.h>
#include <stdio.h>

/* Inter-component Headers */
#include "log.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */

static uint32_t counter = 0;

TASK(hello_task, TASK_STACK_512) {
  while (1) {
    printf("Hello! Counter: %" PRIu32 "\r\n", counter++);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main(void) {
  mcu_init();
  gpio_init();
  tasks_init();
  log_init();

  tasks_init_task(hello_task, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
