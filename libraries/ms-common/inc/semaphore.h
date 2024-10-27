#pragma once

/************************************************************************************************
 * semaphore.h
 *
 * Header file for the semaphore library
 *
 * Created: 2024-10-27
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "semphr.h"

/* Intra-component Headers */
#include "status.h"

#define BLOCK_INDEFINITELY UINT16_MAX

/**
 * @brief Semaphore object with handle and
 */
typedef struct Semaphore {
  SemaphoreHandle_t handle;
  StaticSemaphore_t buffer;
} Semaphore;

typedef Semaphore Mutex;

/**
 * @brief   Initializes a mutex using provided static entity
 * @param   mutex Mutex handle
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_UNINITIALIZED on failure
 */
StatusCode mutex_init(Mutex *mutex);

/**
 * @brief   Locks a Mutex
 * @details The locking task is responsible for unlocking the mutex
 *          Waits ms_to_wait milliseconds if mutex is already locked, then times out
 *          Using BLOCK_INDEFINITELY will cause this method to wait forever on mutex becoming
 *          available
 * @param   mutex Mutex handle
 * @param   ms_to_wait Amoutn of time to wait for mutex
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_TIMEOUT on timeout
 */
StatusCode mutex_lock(Mutex *mutex, uint16_t ms_to_wait);

/**
 * @brief   Unlocks a Mutex
 * @details The task which locks the mutex MUST also be the one to unlock it
 *          Caller specifies whether the method is being called from a task or interrupt handler
 *          higher_priority_task_woken informs the caller whether a higher priority task has become
 *          unblocked
 * @param   mutex Mutex handle
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_INTERNAL_ERROR on failure
 */
StatusCode mutex_unlock(Mutex *mutex);

/**
 * @brief   Initializes counting semaphore with max and initial count
 * @param   sem Semaphore handle
 * @param   max_count Maximum count for counting semaphore
 * @param   initial_count Initial count value to set the counting semaphore
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_UNINITIALIZED on pxSemaphoreBuffer is NULL
 */
StatusCode sem_init(Semaphore *sem, uint32_t max_count, uint32_t initial_count);

/**
 * @brief   Obtains previously initiated semaphore and decrements the counting semaphore
 * @details Waits timeout_ms milliseconds for the semaphore to become available
 * @param   sem Semaphore handle
 * @param   timeout_ms Amount of time to wait for the semaphore
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_TIMEOUT if xTicksToWait expires
 */
StatusCode sem_wait(Semaphore *sem, uint32_t timeout_ms);

/**
 * @brief   Releases a semaphore and increments the counting semaphore
 * @details Note that this must not be used from ISR
 *          An error can occur if there is no space on the queue
 * @param   sem Semaphore handle
 * @return  STATUS_CODE_OK if semaphore is released
 *          STATUS_CODE_INTERNAL_ERROR on error
 */
StatusCode sem_post(Semaphore *sem);
