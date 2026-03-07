/************************************************************************************************
 * @file    xb_transmit.c
 *
 * @brief   XBee transmit header file
 *
 * @date    2026-02-11
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "can.h"
#include "log.h"
#include "tasks.h"
#include "uart.h"

/* Intra-component Headers */
#include "datagram.h"
#include "telemetry.h"
#include "xb_transmit.h"

#define XB_TRANSMIT_DEBUG 0U

static TelemetryStorage *s_telemetry_storage = NULL;

TASK(can_message_forwarder, TASK_STACK_512) {
  size_t datagram_length = 0U;
  Datagram tx_datagram = { 0U };
  CanMessage message = { 0U };
  StatusCode status = STATUS_CODE_OK;

  while (true) {
    /* Wait for new data to be in the queue */
    while (queue_receive(&s_telemetry_storage->can_storage->rx_queue.queue, &message, QUEUE_DELAY_BLOCKING) != STATUS_CODE_OK) {
    }

#if (XB_TRANSMIT_DEBUG == 1)
    LOG_DEBUG("Received message\r\n");
#endif
    decode_can_message(&tx_datagram, &message);

    datagram_length = tx_datagram.dlc + DATAGRAM_METADATA_SIZE;

    status = uart_tx(UART_PORT_2, (uint8_t *)&tx_datagram, datagram_length);
#if (XB_TRANSMIT_DEBUG == 1)
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Failed to transmit to telemetry transceiver!\r\n");
    } else {
      LOG_DEBUG("Transmitted message\r\n");
    }
#endif
  }
}

StatusCode xb_transmit_init(TelemetryStorage *storage, TelemetryConfig *config) {
  if (storage == NULL || config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_telemetry_storage = storage;
  s_telemetry_storage->config = config;
  tasks_init_task(can_message_forwarder, TASK_PRIORITY(2), NULL);

  return STATUS_CODE_OK;
}
