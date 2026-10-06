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
#include "persist.h"
#include "status.h"
#include "system_can.h"
#include "tasks.h"
#include "uart.h"

/* Intra-component Headers */
#include "bmi323.h"
#include "flash.h"
#include "imu.h"
#include "sd_card_interface.h"
#include "telemetry.h"
#include "telemetry_hw_defs.h"
#include "telemetry_log.h"
#include "xb_transmit.h"

static GpioAddress s_telemetry_board_led = GPIO_TELEMETRY_BOARD_LED;
static GpioAddress s_xbee_sleep = GPIO_TELEMETRY_XBEE_SLEEP_RQ;
static GpioAddress s_xbee_reset = GPIO_TELEMETRY_XBEE_XRST;

static PersistStorage persist_storage = { 0U };
static RebootCount reboot_count = { 0U };

static StatusCode s_receive_can(CanMessage *message) {
  /* A full SD queue must not stop normal CAN processing */
  telemetry_log_can(message);
  return ws22_motor_can_process_rx(message->data_u8, message->id.raw, message->dlc);
}

static const CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_TELEMETRY,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = GPIO_TELEMETRY_CAN_TX,
  .rx = GPIO_TELEMETRY_CAN_RX,
  .loopback = false,
  .can_rx_all_cb = s_receive_can,
};

static StatusCode s_record_boot(void) {
  status_ok_or_return(flash_init());

  status_ok_or_return(persist_init(&persist_storage, NUM_FLASH_PAGES - 1U, &reboot_count, sizeof(reboot_count), false));

  if (reboot_count.reboot_number == UINT32_MAX) return STATUS_CODE_OUT_OF_RANGE;
  ++reboot_count.reboot_number;

  return persist_commit(&persist_storage);
}

StatusCode telemetry_init(TelemetryStorage *telemetry_storage, TelemetryConfig *config, Bmi323Storage *bmi323_storage, CanStorage *can_storage) {
  if (telemetry_storage == NULL || config == NULL || bmi323_storage == NULL || can_storage == NULL || bmi323_storage->settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  status_ok_or_return(s_record_boot());
  telemetry_storage->reboot_number = reboot_count;

  telemetry_storage->config = config;
  telemetry_storage->bmi323_storage = bmi323_storage;
  telemetry_storage->can_storage = can_storage;

  status_ok_or_return(uart_init(telemetry_storage->config->uart_port, &telemetry_storage->config->uart_settings));
  status_ok_or_return(can_init(telemetry_storage->can_storage, &s_can_settings));

  telemetry_storage->datagram_queue.item_size = sizeof(Datagram);
  telemetry_storage->datagram_queue.num_items = DATAGRAM_BUFFER_SIZE;
  telemetry_storage->datagram_queue.storage_buf = (uint8_t *)telemetry_storage->datagram_buffer;

  StatusCode queue_status = queue_init(&telemetry_storage->datagram_queue);
  if (queue_status != STATUS_CODE_OK) {
    LOG_DEBUG("Datagram queue initialization failed: %u\n", (unsigned)queue_status);
    return queue_status;
  }

  StatusCode sd_status = sd_card_link_driver(telemetry_storage->config->sd_spi_port, &telemetry_storage->config->sd_spi_settings);

  if (sd_status != STATUS_CODE_OK) {
    LOG_DEBUG("SD driver registration failed: %u\n", (unsigned)sd_status);
    return sd_status;
  }

  FRESULT mount_result = sd_card_mount();

  /* FRESULT -> StatusCode */
  if (mount_result != FR_OK) {
    return mount_result == FR_NOT_READY ? STATUS_CODE_UNINITIALIZED : STATUS_CODE_INTERNAL_ERROR;
  }

  status_ok_or_return(telemetry_log_init(sd_card_drive_path(), reboot_count.reboot_number));

  status_ok_or_return(gpio_init_pin(&s_telemetry_board_led, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH));
  status_ok_or_return(gpio_init_pin(&s_xbee_sleep, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW));
  status_ok_or_return(gpio_init_pin(&s_xbee_reset, GPIO_OUTPUT_OPEN_DRAIN, GPIO_STATE_HIGH));

  return STATUS_CODE_OK;
}
