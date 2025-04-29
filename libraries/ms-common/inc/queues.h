#pragma once

/************************************************************************************************
 * @file   queues.h
 *
 * @brief  Header file for the queue library
 *
 * @date   2024-11-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "queue.h"

/* Intra-component Headers */
#include "status.h"

/**
 * @defgroup RTOS_Helpers
 * @brief    RTOS helper libraries
 * @{
 */

/** @brief  Maximum delay time to block current thread */
#define QUEUE_DELAY_BLOCKING 0xFFFFFFFFU

/**
 * @brief   Queue storage and access struct
 */
typedef struct {
  uint32_t num_items;   /**< Number of items the queue can hold */
  uint32_t item_size;   /**< Size of each item */
  uint8_t *storage_buf; /**< Must be declared statically, and have size num_items*item_size */
  StaticQueue_t queue;  /**< Internal Queue storage */
  QueueHandle_t handle; /**< Handle used for all queue operations */
} Queue;

/**
 * @brief   Create a queue with the parameters specified in settings
 * @param   queue Pointer to queue handle
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode queue_init(Queue *queue);

/**
 * @brief   Place an item into the queue, delaying for delay_ms before timing out
 * @param   queue Pointer to queue handle
 * @param   item Pointer to the item sent to the queue
 * @param   delay_ms Time in milliseconds to wait for space in the queue before timing out
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_RESOURCE_EXHAUSTED if the queue is full
 */
StatusCode queue_send(Queue *queue, const void *item, uint32_t delay_ms);

/**
 * @brief   Place an item into the queue from an ISR
 * @param   queue Pointer to queue handle
 * @param   item Pointer to the item sent to the queue
 * @param   higher_prio_woken Boolean to indicate a context switch after exiting the ISR
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_RESOURCE_EXHAUSTED if the queue is full
 */
StatusCode queue_send_from_isr(Queue *queue, const void *item, BaseType_t *higher_prio_woken);

/**
 * @brief   Retrieve an item from the queue, delaying for delay_ms before timing out
 * @param   queue Pointer to queue handle
 * @param   item Pointer to the buffer to fill from the queue
 * @param   delay_ms Time in milliseconds to wait for data in the queue before timing out
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_RESOURCE_EXHAUSTED if the queue is empty
 */
StatusCode queue_receive(Queue *queue, void *buf, uint32_t delay_ms);

/**
 * @brief   Retrieve an item from the queue, delaying for delay_ms before timing out
 * @param   queue Pointer to queue handle
 * @param   item Pointer to the buffer to fill from the queue
 * @param   higher_prio_woken Boolean to indicate a context switch after exiting the ISR
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_RESOURCE_EXHAUSTED if the queue is empty
 */
StatusCode queue_receive_from_isr(Queue *queue, void *buf, BaseType_t *higher_prio_woken);

// Attempt to receive an item from the queue without removing it from the queue, delaying for
// delay_ms in ms before timing out.

/**
 * @brief   Receive an item from the queue without removing it
 * @param   queue Pointer to queue handle
 * @param   item Pointer to the buffer to fill from the queue
 * @param   delay_ms Time in milliseconds to wait for data in the queue before timing out
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_RESOURCE_EXHAUSTED if the queue is empty
 */
StatusCode queue_peek(Queue *queue, void *buf, uint32_t delay_ms);

/**
 * @brief   Reset all data in the queue
 * @param   queue Pointer to queue handle
 */
void queue_reset(Queue *queue);

/**
 * @brief   Retrieve the total number of spaces in the queue
 * @param   queue Pointer to queue handle
 * @return  Total number of spaces available
 */
uint32_t queue_get_num_items(Queue *queue);

/**
 * @brief   Retrieve the space left in the queue
 * @param   queue Pointer to queue handle
 * @return  Remaining number of spaces available
 */
uint32_t queue_get_spaces_available(Queue *queue);

/** @} */
