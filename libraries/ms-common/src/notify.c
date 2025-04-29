/************************************************************************************************
 * @file   notify.h
 *
 * @brief  Notify Library Header file
 *
 * @date   2024-11-05
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "notify.h"

#include "log.h"

StatusCode event_from_notification(uint32_t *notification, Event *event) {
  if (notification == NULL) {
    *event = INVALID_EVENT;
    return STATUS_CODE_INVALID_ARGS;
  }
  if (*notification == 0) {
    *event = INVALID_EVENT;
    return STATUS_CODE_OK;
  }
  /* Get index of first event */
  *event = 31 - __builtin_clz(*notification);
  /* Clear bit */
  *notification = *notification & ~(1u << *event);

  return STATUS_CODE_INCOMPLETE;
}

bool notify_check_event(uint32_t *notification, Event event) {
  if (!notification || event >= INVALID_EVENT) {
    LOG_DEBUG("Provided bad event\n");
    return false;
  }
  // Check event bit in notification
  if ((1U << event) & *notification) {
    *notification &= ~(1U << event);
    return true;
  } else {
    return false;
  }
}

StatusCode notify_get(uint32_t *notification) {
  return notify_wait(notification, 0U);
}

StatusCode notify_wait(uint32_t *notification, uint32_t ms_to_wait) {
  TickType_t ticks_to_wait = 0U;
  if (ms_to_wait == BLOCK_INDEFINITELY) {
    ticks_to_wait = portMAX_DELAY;
  } else {
    ticks_to_wait = pdMS_TO_TICKS(ms_to_wait);
  }
  /* Block until notification arrives */
  BaseType_t result = xTaskNotifyWait(0, UINT32_MAX, notification, ticks_to_wait);
  if (result) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_TIMEOUT;
  }
}

StatusCode notify(Task *task, Event event) {
  if (event >= INVALID_EVENT) {
    return STATUS_CODE_INVALID_ARGS;
  }

  BaseType_t result = xTaskNotify(task->handle, 1U << event, eSetBits);

  if (result) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_UNKNOWN;
  }
}

void notify_from_isr(Task *task, Event event) {
  BaseType_t task_woken = 0U;
  xTaskNotifyFromISR(task->handle, 1U << event, eSetBits, &task_woken);
  portYIELD_FROM_ISR(task_woken);
}
