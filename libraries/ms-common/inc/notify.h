#pragma once

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
#include "FreeRTOS.h"

/* Intra-component Headers */
#include "status.h"
#include "tasks.h"

/**
 * @defgroup RTOS_Helpers
 * @brief    RTOS helper libraries
 * @{
 */

/* Maximum block amount */
#define BLOCK_INDEFINITELY UINT16_MAX

/* Invalid event ID */
#define INVALID_EVENT 32

/* 8 bit value that represents a bit in the notification */
typedef uint8_t Event;

/**
 * @brief   Get the highest priority event available and clears it
 * @details Call this function in a while loop to clear all events
 * @param   notification Notification value that is cleared and returned
 * @param   event Event value that is updated to the highest priority
 * @return  STATUS_CODE_OK if all events are processed (notification = 0)
 *          STATUS_CODE_INCOPMLETE if some events are not cleared
 */
StatusCode event_from_notification(uint32_t *notification, Event *event);

/**
 * @brief   Checks if the notification is available in the event
 * @param   notification Pointer to the notification to
 * @return  True if the notification exists. False if it does not
 */
bool notify_check_event(uint32_t *notification, Event event);

/**
 * @brief   Get the current notification value for the calilng task without a timeout
 * @param   notification Pointer to a notification value that is updated upon success
 * @return  STATUS_CODE_OK if the value is retrieved successfully
 *          STATUS_CODE_TIMEOUT if the value cannot be retrieved
 */
StatusCode notify_get(uint32_t *notification);

/**
 * @brief   Get the current notification value for the calilng task with a maximum timeout
 * @param   notification Pointer to a notification value that is polled
 * @param   ms_to_wait Time in milliseconds to wait for a notification before timing out
 * @return  STATUS_CODE_OK if notification is received successfully
 *          STATUS_CODE_TIMEOUT if a timeout has occurred
 */
StatusCode notify_wait(uint32_t *notification, uint32_t ms_to_wait);

/**
 * @brief   Notify a specific task with an event
 * @param   task Pointer to the task to notify
 * @param   event Numeric value used to signal task
 * @return  STATUS_CODE_OK
 */
StatusCode notify(Task *task, Event event);

/**
 * @brief   Sends an event notification from an ISR to a task
 * @param   task Pointer to the task to notify
 * @param   event Numeric value used to signal task
 */
void notify_from_isr(Task *task, Event event);

/** @} */
