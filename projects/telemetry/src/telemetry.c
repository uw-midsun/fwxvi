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
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "system_can.h"
#include "tasks.h"
#include "uart.h"

/* Intra-component Headers */
#include "bmi323.h"
#include "telemetry.h"

static TelemetryStorage *telemetry_storage;

static CanStorage s_can_storage = { 0 };
static GpioAddress s_telemetry_board_led = { .port = GPIO_PORT_A, .pin = 6 };

static const CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_TELEMETRY,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_B, 8 },
  .rx = { GPIO_PORT_B, 9 },
  .loopback = false,
  .can_rx_all_cb = NULL,
};

// TASK(can_message_processor, TASK_STACK_512) {
//   Datagram tx_datagram = { 0U };
//   CanMessage message = { 0U };
//   StatusCode status = STATUS_CODE_OK;
//   size_t datagram_length = 0U;
//   uint32_t delay_time_ms = (1U / telemetry_storage->config->message_transmit_frequency_hz) * 1000U;

//   while (true) {
//     while (queue_receive(&s_can_storage.rx_queue.queue, &message, QUEUE_DELAY_BLOCKING) != STATUS_CODE_OK) {
//     }

//     LOG_DEBUG("Received message\n");
//     decode_can_message(&tx_datagram, &message);

//     /* Wait for new data to be in the queue */
//     datagram_length = tx_datagram.dlc + DATAGRAM_METADATA_SIZE;

//     status = uart_tx(UART_PORT_2, (uint8_t *)&tx_datagram, datagram_length);

//     if (status != STATUS_CODE_OK) {
//       LOG_DEBUG("Failed to transmit to telemetry transceiver!\n");
//     }
//   }
// }

StatusCode telemetry_init(TelemetryStorage *telemetry_storage, TelemetryConfig *config, Bmi323Storage *bmi323_storage) {
  if (telemetry_storage == NULL || config == NULL || bmi323_storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  telemetry_storage = telemetry_storage;
  telemetry_storage->config = config;
  // telemetry_storage->bmi323_storage = bmi323_storage;

  // telemetry_storage->datagram_queue.item_size = sizeof(Datagram);
  // telemetry_storage->datagram_queue.num_items = DATAGRAM_BUFFER_SIZE;
  // telemetry_storage->datagram_queue.storage_buf = (uint8_t *)telemetry_storage->datagram_buffer;

  // uart_init(telemetry_storage->config->uart_port, &telemetry_storage->config->uart_settings);
  can_init(&s_can_storage, &s_can_settings);
  // queue_init(&telemetry_storage->datagram_queue);
  // bmi323_init(bmi323_storage);

  // tasks_init_task(can_message_processor, TASK_PRIORITY(2), NULL);
  gpio_init_pin(&s_telemetry_board_led, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  return STATUS_CODE_OK;
}
