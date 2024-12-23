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

/* Intra-component Headers */
#include "can_hw.h"
#include "can.h"

// rx_struct g_rx_struct;
// tx_struct g_tx_struct;

static CanStorage *s_can_storage;

static StaticSemaphore_t s_can_tx_mutex;
static SemaphoreHandle_t s_can_tx_handle;

static StaticSemaphore_t s_can_rx_mutex;
static SemaphoreHandle_t s_can_rx_handle;

StatusCode can_init(CanStorage *storage, const CanSettings *settings)
{
  // if (settings->device_id >= CAN_MSG_MAX_DEVICES) {
  //   return STATUS_CODE_INVALID_ARGS;
  // }

  s_can_storage = storage;

  memset(s_can_storage, 0, sizeof(*s_can_storage));
  s_can_storage->device_id = settings->device_id;

  // memset(&g_tx_struct, 0, sizeof(g_tx_struct));
  // memset(&g_rx_struct, 0, sizeof(g_rx_struct));

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