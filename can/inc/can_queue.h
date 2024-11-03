#pragma once

/************************************************************************************************
 * can_queue.h
 *
 * Wrappers for CAN queues to reduce instruction jumps
 *
 * Created: 2024-11-02
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "queues.h"

/* Intra-component Headers */
#include "can_msg.h"

#define CAN_QUEUE_SIZE 64

typedef struct CanQueue {
  Queue queue;
  CanMessage msg_nodes[CAN_QUEUE_SIZE];
} CanQueue;

#define can_queue_init(can_queue)                                   \
  ({                                                                \
    Queue* queue       = &(can_queue)->queue;                       \
    queue->num_items   = CAN_QUEUE_SIZE;                            \
    queue->item_size   = sizeof(CanMessage);                        \
    queue->storage_buf = (uint8_t*) (&(can_queue)->msg_nodes);      \
    queue_init(&(can_queue)->queue);                                \
  })

#define can_queue_push(can_queue, source)                           \
    queue_send(&(can_queue)->queue, (source), 0)

#define can_queue_push_from_isr(can_queue, source, high_prio_woken) \
    queue_send_from_isr(&(can_queue)->queue, (source), high_prio_woken)

#define can_queue_peek(can_queue, dest)                             \
    queue_peek(&(can_queue)->queue, (dest), 0)

#define can_queue_pop(can_queue, dest)                              \
    queue_receive(&(can_queue)->queue, (dest), 0)

#define can_queue_pop_from_isr(can_queue, dest, higher_prio_woken)  \
    queue_receive_from_isr(&(can_queue)->queue, (dest), high_prio_woken)

#define can_queue_size(can_queue)                                   \
    queue_get_num_items(&(can_queue)->queue)
