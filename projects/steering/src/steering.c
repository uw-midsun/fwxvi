/************************************************************************************************
 * @file   steering.c
 *
 * @brief  Source code for steering system
 *
 * @date   2025-01-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "can.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "system_can.h"

/* Intra-component Headers */
#include "button_manager.h"
#include "drive_state_manager.h"
#include "light_signal_manager.h"
#include "steering.h"
#include "steering_hw_defs.h"

/************************************************************************************************
 * Storage definitions
 ************************************************************************************************/

static SteeringStorage *steering_storage;

static ButtonManager s_button_manager = { 0 };

static CanStorage s_can_storage = { 0 };

/************************************************************************************************
 * Settings definitions
 ************************************************************************************************/

static const CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_STEERING,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = STEERING_CAN_TX,
  .rx = STEERING_CAN_RX,
  .loopback = false,
  .can_rx_all_cb = NULL,
};

StatusCode steering_init(SteeringStorage *storage, SteeringConfig *config) {
  if (storage == NULL || config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  steering_storage = storage;
  steering_storage->config = config;

  log_init();
  can_init(&s_can_storage, &s_can_settings);
  drive_state_manager_init();
  lights_signal_manager_init();
  button_manager_init(&s_button_manager);

  return STATUS_CODE_OK;
}
