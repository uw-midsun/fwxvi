/************************************************************************************************
 * @file    rear_controller.c
 *
 * @brief   Rear controller main source file
 *
 * @date    2025-10-08
 * @author  Midnight Sun Team #24 - MSXVI (revised by hungwn2)
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */
#include "can.h"
#include "flash.h"
#include "log.h"
#include "master_tasks.h"
#include "status.h"
#include "system_can.h"

/* Intra-component Headers */
#include "bps_fault.h"
#include "current_sense.h"
#include "killswitch.h"
#include "power_path_manager.h"
#include "precharge.h"
#include "rear_controller.h"
#include "rear_controller_hw_defs.h"
#include "rear_controller_state_manager.h"
#include "relays.h"

/************************************************************************************************
 * Storage definitions
 ************************************************************************************************/

static RearControllerStorage *rear_controller_storage;

static CanStorage s_can_storage = { 0 };

/************************************************************************************************
 * Settings definitions
 ************************************************************************************************/

static const CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_REAR_CONTROLLER,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = GPIO_REAR_CONTROLLER_CAN_TX,
  .rx = GPIO_REAR_CONTROLLER_CAN_RX,
  .loopback = false,
  .can_rx_all_cb = NULL,
};

static GpioAddress s_rear_controller_board_led = GPIO_REAR_CONTROLLER_BOARD_LED;

StatusCode rear_controller_init(RearControllerStorage *storage, RearControllerConfig *config) {
  if (storage == NULL || config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;
  rear_controller_storage->config = config;

  rear_controller_storage->pos_relay_closed = false;
  rear_controller_storage->neg_relay_closed = false;
  rear_controller_storage->solar_relay_closed = false;
  rear_controller_storage->motor_relay_closed = false;
  rear_controller_storage->motor_lv_enabled = false;
  rear_controller_storage->precharge_complete = false;
  rear_controller_storage->killswitch_active = false;
  rear_controller_storage->pcs_valid = false;
  rear_controller_storage->aux_valid = false;

  /* Initialize hardware peripherals */
  can_init(&s_can_storage, &s_can_settings);
  flash_init();

  /* Initialize rear controller systems */
  relays_init(rear_controller_storage);
  rear_controller_state_manager_init(rear_controller_storage);
  killswitch_init(REAR_CONTROLLER_KILLSWITCH_EVENT, get_1000hz_task());
  precharge_init(REAR_CONTROLLER_PRECHARGE_EVENT, get_10hz_task());
  power_path_manager_init(rear_controller_storage);
  current_sense_init(rear_controller_storage);
  bps_fault_init(rear_controller_storage);

  gpio_init_pin(&s_rear_controller_board_led, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  LOG_DEBUG("Rear controller initialized\r\n");

  return STATUS_CODE_OK;
}
