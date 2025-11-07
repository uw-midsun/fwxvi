/************************************************************************************************
 * @file   soft_timer.c
 *
 * @brief  Source code for the software timer library
 *
 * @date   2025-11-06
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "soft_timer.h"

StatusCode soft_timer_init(uint32_t duration_ms, SoftTimerCallback callback, SoftTimer *timer) {
  if (timer->id != NULL) {
    /* Timer already exist/inuse, delete the old timer */
    xTimerDelete(timer->id, 0);
  }

  timer->id = xTimerCreateStatic(NULL, pdMS_TO_TICKS(duration_ms), pdFALSE,  //
                                 NULL, callback, &timer->buffer);
  return STATUS_CODE_OK;
}

StatusCode soft_timer_start(SoftTimer *timer) {
  if (timer->id == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }
  if (xTimerStart(timer->id, 0) != pdPASS) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  return STATUS_CODE_OK;
}

StatusCode soft_timer_init_and_start(uint32_t duration_ms, SoftTimerCallback callback, SoftTimer *timer) {
  status_ok_or_return(soft_timer_init(duration_ms, callback, timer));
  return soft_timer_start(timer);
}

StatusCode soft_timer_cancel(SoftTimer *timer) {
  if (xTimerDelete(timer->id, 0) != pdPASS) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode soft_timer_reset(SoftTimer *timer) {
  if (xTimerReset(timer->id, 0) != pdPASS) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

bool soft_timer_inuse(SoftTimer *timer) {
  return xTimerIsTimerActive(timer->id);
}

uint32_t soft_timer_remaining_time(SoftTimer *timer) {
  if (!soft_timer_inuse(timer)) {
    return 0;
  }

  /* Convert to ms */
  return (xTimerGetExpiryTime(timer->id) - xTaskGetTickCount()) * 1000U / configTICK_RATE_HZ;
}
