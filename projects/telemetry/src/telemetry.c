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
#include "sd_card_interface.h"
#include "telemetry.h"
#include "telemetry_hw_defs.h"
#include "xb_transmit.h"

static TelemetryStorage *telemetry_storage;
static GpioAddress s_telemetry_board_led = GPIO_TELEMETRY_BOARD_LED;

static const CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_TELEMETRY,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = GPIO_TELEMETRY_CAN_TX,
  .rx = GPIO_TELEMETRY_CAN_RX,
  .loopback = false,
  .can_rx_all_cb = NULL,
};

StatusCode telemetry_init(TelemetryStorage *telemetry_storage, TelemetryConfig *config, Bmi323Storage *bmi323_storage, CanStorage *can_storage) {
  if (telemetry_storage == NULL || config == NULL || bmi323_storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  telemetry_storage = telemetry_storage;
  telemetry_storage->config = config;
  telemetry_storage->bmi323_storage = bmi323_storage;
  telemetry_storage->can_storage = can_storage;

  telemetry_storage->datagram_queue.item_size = sizeof(Datagram);
  telemetry_storage->datagram_queue.num_items = DATAGRAM_BUFFER_SIZE;
  telemetry_storage->datagram_queue.storage_buf = (uint8_t *)telemetry_storage->datagram_buffer;

  uart_init(telemetry_storage->config->uart_port, &telemetry_storage->config->uart_settings);
  can_init(telemetry_storage->can_storage, &s_can_settings);
  queue_init(&telemetry_storage->datagram_queue);
  xb_transmit_init(telemetry_storage);
  bmi323_init(bmi323_storage);
  sd_card_link_driver(telemetry_storage->config->sd_spi_port, &telemetry_storage->config->sd_spi_settings);

  gpio_init_pin(&s_telemetry_board_led, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  return STATUS_CODE_OK;
}
