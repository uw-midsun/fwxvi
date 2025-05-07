/************************************************************************************************
 * @file   delay.c
 *
 * @brief  Source code for the delay library
 *
 * @date   2024-10-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "task.h"

/* Intra-component Headers */
#include "delay.h"
#include "log.h"

void delay_ms(uint32_t time_ms) {
  if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
    LOG_WARN("Scheduler is not running.");
  }
  vTaskDelay(time_ms);
}

void non_blocking_delay_ms(uint32_t time_ms) {
  TickType_t time_ticks = pdMS_TO_TICKS(time_ms) + xTaskGetTickCount();
  while (xTaskGetTickCount() < time_ticks) {
  }
}
