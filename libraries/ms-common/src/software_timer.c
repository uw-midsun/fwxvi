/************************************************************************************************
 * @file   software_timer.c
 *
 * @brief  Source code for the software timer library
 *
 * @date   2025-11-06
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "software_timer.h"

StatusCode software_timer_init(uint32_t duration_ms, SoftTimerCallback callback, SoftTimer *timer) {
  if (timer->id != NULL) {
    /* Timer already exist/inuse, delete the old timer */
    xTimerDelete(timer->id, 0);
  }

  timer->id = xTimerCreateStatic(NULL, pdMS_TO_TICKS(duration_ms), pdFALSE,  //
                                 NULL, callback, &timer->buffer);
  return STATUS_CODE_OK;
}

StatusCode software_timer_start(SoftTimer *timer) {
  if (timer->id == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }
  if (xTimerStart(timer->id, 0) != pdPASS) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  return STATUS_CODE_OK;
}

StatusCode software_timer_init_and_start(uint32_t duration_ms, SoftTimerCallback callback, SoftTimer *timer) {
  status_ok_or_return(software_timer_init(duration_ms, callback, timer));
  return software_timer_start(timer);
}

StatusCode software_timer_cancel(SoftTimer *timer) {
  if (xTimerDelete(timer->id, 0) != pdPASS) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode software_timer_reset(SoftTimer *timer) {
  if (xTimerReset(timer->id, 0) != pdPASS) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

bool software_timer_inuse(SoftTimer *timer) {
  return xTimerIsTimerActive(timer->id);
}

uint32_t software_timer_remaining_time(SoftTimer *timer) {
  if (!software_timer_inuse(timer)) {
    return 0;
  }

  /* Convert to ms */
  return (xTimerGetExpiryTime(timer->id) - xTaskGetTickCount()) * 1000U / configTICK_RATE_HZ;
}

StatusCode software_watchdog_init(SoftwareWatchdog *watchdog, uint32_t period_ms, SoftTimerCallback fault_callback) {
  if (watchdog == NULL || fault_callback == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  watchdog->period_ms = period_ms;
  watchdog->fault_callback = fault_callback;

  /* Use soft_timer to implement the watchdog */
  return software_timer_init_and_start(period_ms, fault_callback, &watchdog->timer);
}

StatusCode software_watchdog_kick(SoftwareWatchdog *watchdog) {
  if (watchdog == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return software_timer_reset(&watchdog->timer);
}

StatusCode software_watchdog_stop(SoftwareWatchdog *watchdog) {
  if (watchdog == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return software_timer_cancel(&watchdog->timer);
}
