#pragma once

/************************************************************************************************
 * @file   tasks.h
 *
 * @brief  Header file for the RTOS tasks wrapper
 *
 * @date   2024-10-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

/* Intra-component Headers */
#include "status.h"

/**
 * @defgroup RTOS_Helpers
 * @brief    RTOS helper libraries
 * @{
 */

/**
 * @brief   Define a task function. This should go in a source file (.c).
 * @details The generated function has the following signature:
 *          void _s_your_task_function(void *context)
 *          where context is the context pointer passed to tasks_init_task.
 * @param   task_name is the name of your task, which should match any previous DECLARE_TASK
 * declarations.
 * @param   task_stack_size is the depth of your task's stack - use your judgement to choose.
 */
#define TASK(task_name, task_stack_size)                                                                                                                                                    \
  /* forward declaration so we can reference it in the Task */                                                                                                                              \
  static void _s_task_impl_##task_name(void *);                                                                                                                                             \
  static StackType_t _s_stack_##task_name[task_stack_size];                                                                                                                                 \
  /* use a compound literal so users can use it as a pointer */                                                                                                                             \
  Task *task_name = &((Task){                                                                                                                                                               \
      .task_func = _s_task_impl_##task_name, .name = #task_name, .stack = _s_stack_##task_name, .stack_size = task_stack_size, .handle = NULL, /* will be initialized by tasks_init_task */ \
  });                                                                                                                                                                                       \
  static void _s_task_impl_##task_name(void *context)

/**
 * @brief Maximum amount of RTOS tasks supported at a time
 */
#define MAX_NUM_TASKS 15

/**
 * @brief   Convienience priority to make code more readable
 */
#define TASK_PRIORITY(prio) ((TaskPriority)prio)

/**
 * @brief   Define wait timeout as 1 second, tasks cycle executino should not take longer than a
 * second
 */
#define WAIT_TASK_TIMEOUT_MS 1000

/**
 * @brief Minimum stack size a task can have. If you pass in something smaller than the min size, a
 *        warning will be issued
 */
#define TASK_MIN_STACK_SIZE configMINIMAL_STACK_SIZE

/**
 * @brief Common stack sizes. If your task is failing for strange reasons, bump the stack
 *        size one size up
 */
#define TASK_STACK_256 ((size_t)256)
#define TASK_STACK_512 ((size_t)512)
#define TASK_STACK_1024 ((size_t)1024)
#define TASK_STACK_2048 ((size_t)2048)
#define TASK_STACK_4096 ((size_t)4096)

typedef UBaseType_t TaskPriority;

typedef struct Task {
  TaskHandle_t handle;
  TaskFunction_t task_func;
  char *name;
  StackType_t *stack;
  size_t stack_size;
  StaticTask_t tcb;
  void *context;
} Task;

/**
 * @brief   Initialize a task that was previously declared with TASK().
 * @param   task Task handle
 * @param   priority The task priority: higher number is higher priority, and the maximum
 * @param   context Pointer to arguments that are passed to the task
 *          is configNUM_TASK_PRIORITIES - 1.
 * @return  STATUS_CODE_OK if successfully initialize task.
 */
StatusCode tasks_init_task(Task *task, TaskPriority priority, void *context);

/**
 * @brief   Start the FreeRTOS scheduler to run the tasks that were previously initialized.
 * @details This function should not return!
 */
void tasks_start(void);

/**
 * @brief   Initialize the task module
 * @details Must be called before tasks are initialized or the scheduler is started.
 * @return  STATUS_CODE_OK if successfully initialize the module.
 */
StatusCode tasks_init(void);

/**
 * @brief   Waits for num_tasks of RTOS tasks to finish
 * @param   num_tasks Number of task completions to wait for
 * @details Takes num_tasks from end task semaphore, tracking the number of tasks completed
 * @return  STATUS_CODE_OK if num_tasks items taken successfully
 *          STATUS_CODE_TIMEOUT if timeout occurs
 */
StatusCode wait_tasks(uint16_t num_tasks);

/**
 * @brief   A wrapper to give to the semaphore, to be called by tasks when they complete
 * @return  STATUS_CODE_OK if successfully release sempahore
 */
StatusCode send_task_end(void);

/** @} */
