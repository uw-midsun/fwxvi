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
#include "FreeRTOS.h"
#include "mcu.h"
#include "task.h"

/* Intra-component Headers */

/* Static allocation for task */
#define HELLO_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
static StackType_t helloTaskStack[HELLO_TASK_STACK_SIZE];
static StaticTask_t helloTaskTCB;
static TaskHandle_t helloTaskHandle = NULL;

/* Static allocation for idle task */
static StaticTask_t idleTaskTCB;
static StackType_t idleTaskStack[configMINIMAL_STACK_SIZE];

/* Static allocation for timer task if configUSE_TIMERS is enabled */
#if configUSE_TIMERS == 1
static StaticTask_t timerTaskTCB;
static StackType_t timerTaskStack[configTIMER_TASK_STACK_DEPTH];
#endif

static uint32_t counter = 0;

void vHelloTask(void *pvParameters) {
  while (1) {
    printf("Hello! Counter: %" PRIu32 "\r\n", counter++);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/* Main function */
int main(void) {
  /* Create the task using static allocation */
  helloTaskHandle = xTaskCreateStatic(vHelloTask,            /* Task function */
                                      "HelloTask",           /* Task name */
                                      HELLO_TASK_STACK_SIZE, /* Stack size */
                                      NULL,                  /* Parameters */
                                      1,                     /* Priority */
                                      helloTaskStack,        /* Stack buffer */
                                      &helloTaskTCB);        /* TCB buffer */

  /* Verify task creation */
  if (helloTaskHandle == NULL) {
    /* Handle task creation error */
    for (;;);
  }

  /* Start the scheduler */
  vTaskStartScheduler();

  /* Should never reach here */
  for (;;);
  return 0;
}
