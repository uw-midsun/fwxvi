#pragma once

/************************************************************************************************
 * @file   semaphore.h
 *
 * @brief  Header file for the semaphore library
 *
 * @date   2024-10-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "semphr.h"

/* Intra-component Headers */
#include "status.h"

/**
 * @defgroup RTOS_Helpers
 * @brief    RTOS helper libraries
 * @{
 */

/** @brief Maximum time permitted to wait for a semaphore or mutex */
#define BLOCK_INDEFINITELY UINT16_MAX

/** @brief   Semaphore object with handle and buffer */
typedef struct Semaphore {
  SemaphoreHandle_t handle; /**< Handle to interact with the semaphore */
  StaticSemaphore_t buffer; /**< Stores the current state of the semaphore */
} Semaphore;

typedef Semaphore Mutex;

/**
 * @brief   Initializes a mutex using provided static entity
 * @param   mutex Mutex handle
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_UNINITIALIZED on failure
 */
StatusCode ms_mutex_init(Mutex *mutex);

/**
 * @brief   Locks a Mutex
 * @details The locking task is responsible for unlocking the mutex
 *          Waits ms_to_wait milliseconds if mutex is already locked, then times out
 *          Using BLOCK_INDEFINITELY will cause this method to wait forever on mutex becoming
 *          available
 * @param   mutex Mutex handle
 * @param   ms_to_wait Amount of time to wait for mutex
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_TIMEOUT on timeout
 */
StatusCode ms_mutex_lock(Mutex *mutex, uint16_t ms_to_wait);

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
StatusCode ms_mutex_unlock(Mutex *mutex);

/**
 * @brief   Initializes counting semaphore with max and initial count
 * @param   sem Semaphore handle
 * @param   max_count Maximum count for counting semaphore
 * @param   initial_count Initial count value to set the counting semaphore
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_UNINITIALIZED on pxSemaphoreBuffer is NULL
 */
StatusCode ms_sem_init(Semaphore *sem, uint32_t max_count, uint32_t initial_count);

/**
 * @brief   Obtains previously initiated semaphore and decrements the counting semaphore
 * @details Waits timeout_ms milliseconds for the semaphore to become available
 * @param   sem Semaphore handle
 * @param   timeout_ms Amount of time to wait for the semaphore
 * @return  STATUS_CODE_OK on success
 *          STATUS_CODE_TIMEOUT if xTicksToWait expires
 */
StatusCode ms_sem_wait(Semaphore *sem, uint32_t timeout_ms);

/**
 * @brief   Releases a semaphore and increments the counting semaphore
 * @details Note that this must not be used from ISR
 *          An error can occur if there is no space on the queue
 * @param   sem Semaphore handle
 * @return  STATUS_CODE_OK if semaphore is released
 *          STATUS_CODE_INTERNAL_ERROR on error
 */
StatusCode ms_sem_post(Semaphore *sem);

/** @} */
