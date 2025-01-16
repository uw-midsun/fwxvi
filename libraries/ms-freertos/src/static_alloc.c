/************************************************************************************************
 * @file   static_alloc.c
 *
 * @brief  Static allocation helpers for FreeRTOS.
 *
 * @date   2024-10-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "task.h"

/* Intra-component Headers */

#if (configSUPPORT_STATIC_ALLOCATION == 1)

/**
 * This is needed handle stack overflows.
 * See
 * https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/09-Memory-management/02-Stack-usage-and-stack-overflow-checking
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {}

/**
 * This is needed to handle memory allocation failures during task memory allocation.
 */
void vApplicationMallocFailedHook(void) {
  taskDISABLE_INTERRUPTS();
  for (;;);
}

#endif
