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
#include "event_groups.h"
#include "imu.h"
#include "sd_card_interface.h"
#include "telemetry.h"
#include "telemetry_hw_defs.h"
#include "xb_transmit.h"

#define TELEMETRY_CHECK(stage, operation)                              \
  do {                                                                 \
    StatusCode result = (operation);                                   \
    if (result != STATUS_CODE_OK) {                                    \
      LOG_DEBUG("Telemetry %s failed: %u\n", stage, (unsigned)result); \
      return result;                                                   \
    }                                                                  \
  } while (0)

static GpioAddress s_telemetry_board_led = GPIO_TELEMETRY_BOARD_LED;
static GpioAddress s_xbee_sleep = GPIO_TELEMETRY_XBEE_SLEEP_RQ;
static GpioAddress s_xbee_reset = GPIO_TELEMETRY_XBEE_XRST;

static const CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_TELEMETRY,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = GPIO_TELEMETRY_CAN_TX,
  .rx = GPIO_TELEMETRY_CAN_RX,
  .loopback = false,
  .can_rx_all_cb = NULL,
};

StatusCode telemetry_init(TelemetryStorage *telemetry_storage, TelemetryConfig *config, Bmi323Storage *bmi323_storage, CanStorage *can_storage) {
  if (telemetry_storage == NULL || config == NULL || bmi323_storage == NULL || can_storage == NULL || bmi323_storage->settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  telemetry_storage->config = config;
  telemetry_storage->bmi323_storage = bmi323_storage;
  telemetry_storage->can_storage = can_storage;

  TELEMETRY_CHECK("UART", uart_init(telemetry_storage->config->uart_port, &telemetry_storage->config->uart_settings));
  TELEMETRY_CHECK("CAN", can_init(telemetry_storage->can_storage, &s_can_settings));

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

  if (mount_result != FR_OK) {
    /* sd_card_mount() already logs the exact FatFs error. */
    return mount_result == FR_NOT_READY ? STATUS_CODE_UNINITIALIZED : STATUS_CODE_INTERNAL_ERROR;
  }

  TELEMETRY_CHECK("IMU", imu_init(bmi323_storage, bmi323_storage->settings));

  TELEMETRY_CHECK("LED GPIO", gpio_init_pin(&s_telemetry_board_led, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH));
  TELEMETRY_CHECK("XBee sleep GPIO", gpio_init_pin(&s_xbee_sleep, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW));
  TELEMETRY_CHECK("XBee reset GPIO", gpio_init_pin(&s_xbee_reset, GPIO_OUTPUT_OPEN_DRAIN, GPIO_STATE_HIGH));

  return STATUS_CODE_OK;
}

#define APP_READY_BIT (1U << 0)
static StaticEventGroup_t s_ready_storage;
static EventGroupHandle_t s_ready_event;

StatusCode telemetry_readiness_init(void) {
  s_ready_event = xEventGroupCreateStatic(&s_ready_storage);
  return s_ready_event ? STATUS_CODE_OK : STATUS_CODE_RESOURCE_EXHAUSTED;
}

void telemetry_wait_ready(void) {
  configASSERT(s_ready_event != NULL);
  xEventGroupWaitBits(s_ready_event, APP_READY_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
}

void master_wait_ready(void) {
  telemetry_wait_ready();
}

void telemetry_set_ready(void) {
  xEventGroupSetBits(s_ready_event, APP_READY_BIT);
}
