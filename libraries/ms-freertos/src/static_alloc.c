/************************************************************************************************
 * static_alloc.c
 *
 * Static allocation helpers for FreeRTOS.
 * 
 * Created: 2024-10-27
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <stdint.h>

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "task.h"

/* Intra-component Headers */

#if (configSUPPORT_STATIC_ALLOCATION == 1)

/**
 * This is needed handle stack overflows.
 * See https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/09-Memory-management/02-Stack-usage-and-stack-overflow-checking
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  (void)xTask;
  (void)pcTaskName;
  taskDISABLE_INTERRUPTS();
  for (;;);
}

/**
 * This is needed to handle memory allocation failures during task memory allocation.
 */
void vApplicationMallocFailedHook(void) {
    taskDISABLE_INTERRUPTS();
    for (;;);
}

#endif
