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
#include "uart.h"

/* Intra-component Headers */
#include "datagram.h"
#include "telemetry.h"
#include "xb_transmit.h"

static TelemetryStorage *s_telemetry_storage = NULL;
static size_t datagram_length = 0U;
static Datagram tx_datagram = { 0U };
static CanMessage message = { 0U };

StatusCode xb_transmit_run() {
  datagram_length = 0U;

  if (queue_receive(&s_telemetry_storage->can_storage->rx_queue.queue, &message, QUEUE_DELAY_BLOCKING) == STATUS_CODE_OK) {
    LOG_DEBUG("Received message\n");
    decode_can_message(&tx_datagram, &message);

    /* Wait for new data to be in the queue */
    datagram_length = tx_datagram.dlc + DATAGRAM_METADATA_SIZE;

    StatusCode status = uart_tx(UART_PORT_2, (uint8_t *)&tx_datagram, datagram_length);

    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Failed to transmit to telemetry transceiver!\n");
    }
  }

  return STATUS_CODE_OK;
}

StatusCode xb_transmit_init(TelemetryStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_telemetry_storage = storage;

  return STATUS_CODE_OK;
}
