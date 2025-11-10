#pragma once

/************************************************************************************************
 * @file   software_timer.h
 *
 * @brief  Header file for the software timer library
 *
 * @date   2025-11-06
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "FreeRTOS.h"
#include "status.h"
#include "timers.h"

/**
 * @defgroup RTOS_Helpers
 * @brief    RTOS helper libraries
 * @{
 */

/** @brief  Redefine FreeRTOS TimerHandle_t to SoftTimerId */
typedef TimerHandle_t SoftTimerId;

/** @brief  Software timer callback function */
typedef void (*SoftTimerCallback)(SoftTimerId id);

/**
 * @brief   Soft timer storage
 */
typedef struct {
  StaticTimer_t buffer;
  SoftTimerId id;
} SoftTimer;

/**
 * @brief   Software Watchdog Timer structure
 *          Uses a SoftTimer internally to track timeouts.
 */
typedef struct {
  SoftTimer timer;                     // Internal soft timer used for timing
  uint32_t period_ms;                  // Watchdog timeout period (in milliseconds)
  SoftTimerCallback fault_callback;    // Called when watchdog expires (not kicked in time)
} SoftwareWatchdog;

/** @brief  Software timer callback function */
typedef void (*SoftTimerCallback)(SoftTimerId id);

/**
 * @brief   Creates a new software timer without starting it
 * @param   duration_ms Duration of the timer in milliseconds
 * @param   callback Callback function to execute when the timer expires
 * @param   timer Pointer to the timer instance
 * @return  STATUS_CODE_OK if the timer was successfully initialized
 */
StatusCode software_timer_init(uint32_t duration_ms, SoftTimerCallback callback, SoftTimer *timer);

/**
 * @brief   Starts an initialized software timer
 * @param   timer Pointer to the timer instance
 * @return  STATUS_CODE_OK if the timer was successfully started
 */
StatusCode software_timer_start(SoftTimer *timer);

/**
 * @brief   Initializes and starts a software timer
 * @param   duration_ms Duration of the timer in milliseconds
 * @param   callback Callback function to execute when the timer expires
 * @param   timer Pointer to the timer instance
 * @return  STATUS_CODE_OK if the timer was successfully initialized and started
 */
StatusCode software_timer_init_and_start(uint32_t duration_ms, SoftTimerCallback callback, SoftTimer *timer);

/**
 * @brief   Cancels a running software timer
 * @param   timer Pointer to the timer instance
 * @return  STATUS_CODE_OK if the timer was successfully cancelled
 */
StatusCode software_timer_cancel(SoftTimer *timer);

/**
 * @brief   Restarts a software timer
 * @param   timer Pointer to the timer instance
 * @return  STATUS_CODE_OK if the timer was successfully restarted
 */
StatusCode software_timer_reset(SoftTimer *timer);

/**
 * @brief   Checks if a software timer is currently active
 * @param   timer Pointer to the timer instance
 * @return  true if the timer is running, false otherwise
 */
bool software_timer_inuse(SoftTimer *timer);

/**
 * @brief   Gets the remaining time on a software timer
 * @param   timer Pointer to the timer instance
 * @return  Remaining time in ticks, or 0 if the timer has expired
 */
uint32_t software_timer_remaining_time(SoftTimer *timer);

/**
 * @brief   Initializes the software watchdog timer
 * @param   watchdog Pointer to the watchdog instance
 * @param   period_ms Timeout period in milliseconds
 * @param   fault_callback Function to call if watchdog is not kicked before timeout
 * @return  STATUS_CODE_OK if successfully initialized
 */
StatusCode software_watchdog_init(SoftwareWatchdog *watchdog, uint32_t period_ms,
                                  SoftTimerCallback fault_callback);

/**
 * @brief   "Kicks" (resets) the watchdog timer to prevent timeout
 * @param   watchdog Pointer to the watchdog instance
 * @return  STATUS_CODE_OK if successfully kicked
 */
StatusCode software_watchdog_kick(SoftwareWatchdog *watchdog);

/**
 * @brief   Stops and disables the watchdog timer
 * @param   watchdog Pointer to the watchdog instance
 * @return  STATUS_CODE_OK if successfully stopped
 */
StatusCode software_watchdog_stop(SoftwareWatchdog *watchdog);


/** @} */
