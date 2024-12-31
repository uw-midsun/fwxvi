/************************************************************************************************
 * @file   can.c
 *
 * @brief  Source file for CAN Application code
 *
 * @date   2024-11-23
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>
#include <string.h>

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "semphr.h"
#include "log.h"
#include "system_can.h"
#include "can_codegen.h"

/* Intra-component Headers */
#include "can_hw.h"
#include "can.h"
#include "can_watchdog.h"

rx_struct g_rx_struct;
tx_struct g_tx_struct;

static CanStorage *s_can_storage;

static StaticSemaphore_t s_can_tx_mutex;
static SemaphoreHandle_t s_can_tx_handle;

static StaticSemaphore_t s_can_rx_mutex;
static SemaphoreHandle_t s_can_rx_handle;

StatusCode can_init(CanStorage *storage, const CanSettings *settings)
{
  if (settings->device_id >= NUM_SYSTEM_CAN_DEVICES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_can_storage = storage;

  memset(s_can_storage, 0, sizeof(*s_can_storage));
  s_can_storage->device_id = settings->device_id;

  memset(&g_tx_struct, 0, sizeof(g_tx_struct));
  memset(&g_rx_struct, 0, sizeof(g_rx_struct));

  status_ok_or_return(can_queue_init(&s_can_storage->rx_queue));
 
  /* Initialize CAN HW interface */
  status_ok_or_return(can_hw_init(&s_can_storage->rx_queue, settings));

  s_can_rx_handle = xSemaphoreCreateMutexStatic(&s_can_rx_mutex);
  s_can_tx_handle = xSemaphoreCreateMutexStatic(&s_can_tx_mutex);

  if (s_can_rx_handle == NULL || s_can_tx_handle == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode can_add_filter_in(CanMessageId msg_id) {

  if (s_can_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  } else if (msg_id >= CAN_MSG_MAX_STD_IDS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Set the filter as the message ID */
  CanId can_id = { .raw = msg_id };

  /* Mask everything, to check all bits of the incoming ID */
  CanId mask = { 0U };
  mask.raw = (uint32_t)~mask.msg_id;

  return can_hw_add_filter_in(mask.raw, can_id.raw, false);
}

StatusCode can_transmit(const CanMessage *msg) {
  if (s_can_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  return can_hw_transmit(msg->id.raw, msg->extended, msg->data_u8, msg->dlc);
}

StatusCode can_receive(const CanMessage *msg) {
  if (s_can_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  StatusCode ret = can_queue_pop(&s_can_storage->rx_queue, msg);

  // if (ret == STATUS_CODE_OK)
  // {
  //   LOG_DEBUG("Source Id: %d\n", msg->id);
  //   LOG_DEBUG("Data: %lx\n", msg->data);
  //   LOG_DEBUG("DLC: %ld\n", msg->dlc);
  //   LOG_DEBUG("ret: %d\n", ret);
  // }

  return ret;
}

StatusCode run_can_tx_all() {
  xSemaphoreTake(s_can_tx_handle, pdMS_TO_TICKS(CAN_TIMEOUT_MS));

  can_tx_all();
  
  /* Check all cycles watchdogs, and update internal states */
  check_all_can_watchdogs();
  clear_all_rx_received();

  xSemaphoreGive(s_can_tx_handle);
  return STATUS_CODE_OK;
}

StatusCode run_can_tx_fast() {
  xSemaphoreTake(s_can_tx_handle, pdMS_TO_TICKS(CAN_TIMEOUT_MS));

  can_tx_fast_cycle();

  /* Check fast cycle watchdogs, and update internal states */
  check_fast_can_watchdogs();
  clear_fast_rx_received();

  xSemaphoreGive(s_can_tx_handle);
  return STATUS_CODE_OK;
}

StatusCode run_can_tx_medium() {
  xSemaphoreTake(s_can_tx_handle, pdMS_TO_TICKS(CAN_TIMEOUT_MS));

  can_tx_medium_cycle();

  /* Check medium cycle watchdogs, and update internal states */
  check_medium_can_watchdogs();
  clear_medium_rx_received();

  xSemaphoreGive(s_can_tx_handle);
  return STATUS_CODE_OK;
}

StatusCode run_can_tx_slow() {
  xSemaphoreTake(s_can_tx_handle, pdMS_TO_TICKS(CAN_TIMEOUT_MS));

  can_tx_slow_cycle();

  /* Check slow cycle watchdogs, and update internal states */
  check_slow_can_watchdogs();
  clear_slow_rx_received();

  xSemaphoreGive(s_can_tx_handle);
  return STATUS_CODE_OK;
}

StatusCode run_can_rx_all() {
  xSemaphoreTake(s_can_rx_handle, pdMS_TO_TICKS(CAN_TIMEOUT_MS));
  can_rx_all();
  xSemaphoreGive(s_can_rx_handle);
  return STATUS_CODE_OK;
}

StatusCode clear_rx_struct() {
  memset(&g_rx_struct, 0, sizeof(g_rx_struct));
  return STATUS_CODE_OK;
}

StatusCode clear_tx_struct() {
  memset(&g_rx_struct, 0, sizeof(g_rx_struct));
  return STATUS_CODE_OK;
}
