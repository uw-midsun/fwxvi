/* Include necessary headers */
#include <inttypes.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

/* Task handle */
TaskHandle_t helloTaskHandle = NULL;

/* Task function prototype */
void vHelloTask(void *pvParameters);

/* Counter variable */
static uint32_t counter = 0;

/* Hello task implementation */
void vHelloTask(void *pvParameters) {
  /* Task loop */
  while (1) {
    printf("Hello! Counter: %" PRIu32 "\r\n", counter++);

    /* Delay for 1000ms (1 second) */
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/* Main function */
int main(void) {
  /* Create the task */
  xTaskCreate(vHelloTask,               /* Task function */
              "HelloTask",              /* Task name (for debugging) */
              configMINIMAL_STACK_SIZE, /* Stack size */
              NULL,                     /* Parameters passed to task */
              1,                        /* Task priority */
              &helloTaskHandle          /* Task handle */
  );

  /* Start the scheduler */
  vTaskStartScheduler();

  /* Should never reach here */
  while (1) {
  }

  return 0;
}

/* Required for FreeRTOS */
void vApplicationMallocFailedHook(void) {
  /* Called if a memory allocation fails */
  taskDISABLE_INTERRUPTS();
  for (;;);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  /* Called if a stack overflow is detected */
  (void)xTask;
  (void)pcTaskName;
  taskDISABLE_INTERRUPTS();
  for (;;);
}

void vApplicationIdleHook(void) {
  /* Called when idle */
}

void vApplicationTickHook(void) {
  /* Called for each RTOS tick */
}