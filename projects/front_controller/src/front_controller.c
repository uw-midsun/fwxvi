/************************************************************************************************
 * @file   front_controller.h
 *
 * @brief  Header file for front_controller
 *
 * @date   2025-07-19
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "adc.h"
#include "can.h"
#include "dac.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "opamp.h"
#include "system_can.h"

/* Intra-component Headers */
#include "accel_pedal.h"
#include "front_controller.h"
#include "front_controller_hw_defs.h"
#include "pedal_calib_reader.h"
#include "ws22_motor_can.h"

/************************************************************************************************
 * Storage definitions
 ************************************************************************************************/

static FrontControllerStorage *front_controller_storage;

static CanStorage s_can_storage = { 0 };

/************************************************************************************************
 * Settings definitions
 ************************************************************************************************/

static const CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_FRONT_CONTROLLER,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = FRONT_CONTROLLER_CAN_TX,
  .rx = FRONT_CONTROLLER_CAN_RX,
  .loopback = false,
  .can_rx_all_cb = ws22_motor_can_process_rx,
};

StatusCode front_controller_init(FrontControllerStorage *storage, FrontControllerConfig *config) {
  if (storage == NULL || config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  front_controller_storage = storage;
  front_controller_storage->config = config;

  log_init();

  /* Initialize hardware peripherals */
  can_init(&s_can_storage, &s_can_settings);
  flash_init();
  opamp_init();
  dac_init();
  adc_init();

  /* Initialize front controller systems */
  accel_pedal_init(storage);
  ws22_motor_can_init(storage);
  pedal_calib_read(storage);

  return STATUS_CODE_OK;
}
