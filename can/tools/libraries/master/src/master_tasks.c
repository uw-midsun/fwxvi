/************************************************************************************************
 * @file   master_tasks.c
 *
 * @brief  Source file for Master Tasks API. Supports 1KHz, 100Hz and 10Hz scheduling
 *
 * @date   2024-11-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "master_tasks.h"

#define MASTER_1000HZ_TO_MS 1U
#define MASTER_10HZ_TO_MS 100U
#define MASTER_1HZ_TO_MS 1000U

#define MASTER_TASKS_PRIORITY (configMAX_PRIORITIES - 1U)
#define MAX_CYCLES_OVER 5

static uint8_t s_cycles_over = 0;

void master_no_op() {}

void run_1000hz_cycle() __attribute__((weak, alias("master_no_op")));
void run_10hz_cycle() __attribute__((weak, alias("master_no_op")));
void run_1hz_cycle() __attribute__((weak, alias("master_no_op")));
void pre_loop_init() __attribute__((weak, alias("master_no_op")));

void check_late_cycle(Task *task, BaseType_t delay) {
  if (delay != pdTRUE) {
    ++s_cycles_over;
    LOG_WARN("%s out of sync!! Currently over by %u cycles\n", task->name, s_cycles_over);
  } else {
    if (s_cycles_over != 0) {
      --s_cycles_over;
    }
  }
  if (s_cycles_over > MAX_CYCLES_OVER) {
    LOG_CRITICAL("%s out of sync!! Ending Scheduler\n", task->name);
    vTaskEndScheduler();
  }
}

TASK(master_task_1000hz, MASTER_TASK_1000HZ_SIZE) {
  pre_loop_init();
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (true) {
    run_1000hz_cycle();
    BaseType_t delay = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(MASTER_1000HZ_TO_MS));
    check_late_cycle(master_task_1000hz, delay);
  }
}

TASK(master_task_10hz, MASTER_TASK_10HZ_SIZE) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (true) {
    run_10hz_cycle();
    BaseType_t delay = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(MASTER_10HZ_TO_MS));
    check_late_cycle(master_task_10hz, delay);
  }
}

TASK(master_task_1hz, MASTER_TASK_1HZ_SIZE) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (true) {
    run_1hz_cycle();
    BaseType_t delay = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(MASTER_1HZ_TO_MS));
    check_late_cycle(master_task_1hz, delay);
  }
}

StatusCode init_master_tasks() {
  s_cycles_over = 0;
  status_ok_or_return(tasks_init_task(master_task_1000hz, MASTER_TASKS_PRIORITY, NULL));
  status_ok_or_return(tasks_init_task(master_task_10hz, MASTER_TASKS_PRIORITY - 1U, NULL));
  status_ok_or_return(tasks_init_task(master_task_1hz, MASTER_TASKS_PRIORITY - 2U, NULL));

  return STATUS_CODE_OK;
}

Task *get_1000hz_task() {
  return master_task_1000hz;
}

Task *get_10hz_task() {
  return master_task_10hz;
}

Task *get_1hz_task() {
  return master_task_1hz;
}
