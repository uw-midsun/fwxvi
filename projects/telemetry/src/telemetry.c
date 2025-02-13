/************************************************************************************************
 * @file   telemetry.c
 *
 * @brief  Source code for telemetry system
 *
 * @date   2025-01-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "can.h"
#include "datagram.h"
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "system_can.h"
#include "tasks.h"
#include "uart.h"

/* Intra-component Headers */
#include "telemetry.h"

static TelemetryStorage *telemetry_storage;

static CanStorage s_can_storage = { 0 };
static const CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_TELEMETRY,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

TASK(can_message_listener, TASK_STACK_256) {
  CanMessage message = { 0U };
  Datagram datagram = { 0U };
  StatusCode status = STATUS_CODE_OK;

  while (true) {
    while (queue_receive(&s_can_storage.rx_queue.queue, &message, QUEUE_DELAY_BLOCKING) != STATUS_CODE_OK) {}

    decode_can_message(&datagram, &message);

    /* Push the message to Queue */
    status = queue_send(&telemetry_storage->datagram_queue, &datagram, 0U);

    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Failed to enqueue datagram: %d\n", status);
    }
  }
}

TASK(can_message_processor, TASK_STACK_256) {
  Datagram tx_datagram = { 0U };
  StatusCode status = STATUS_CODE_OK;

  while (true) {
    /* Wait for new data to be in the queue */
    while (queue_receive(&telemetry_storage->datagram_queue, &tx_datagram, QUEUE_DELAY_BLOCKING) == STATUS_CODE_OK) {
      status = uart_tx(UART_PORT_2, (uint8_t *)&tx_datagram, tx_datagram.dlc + DATAGRAM_METADATA_SIZE);

      if (status != STATUS_CODE_OK) {
        LOG_DEBUG("Failed to transmit to telemetry transceiver!\n");
      }
    }
  }
}

StatusCode telemetry_init(TelemetryStorage *storage, TelemetryConfig *config) {
  if (storage == NULL || config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  telemetry_storage = storage;
  telemetry_storage->config = config;

  telemetry_storage->datagram_queue.item_size = sizeof(Datagram);
  telemetry_storage->datagram_queue.num_items = DATAGRAM_BUFFER_SIZE;
  telemetry_storage->datagram_queue.storage_buf = (uint8_t *)telemetry_storage->datagram_buffer;

  log_init();
  uart_init(telemetry_storage->config->uart_port, &telemetry_storage->config->uart_settings);
  can_init(&s_can_storage, &s_can_settings);
  queue_init(&telemetry_storage->datagram_queue);

  tasks_init_task(can_message_listener, TASK_PRIORITY(2), NULL);
  tasks_init_task(can_message_processor, TASK_PRIORITY(2), NULL);

  return STATUS_CODE_OK;
}
