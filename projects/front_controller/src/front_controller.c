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
#include "flash.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "opamp.h"
#include "system_can.h"

/* Intra-component Headers */
#include "accel_pedal.h"
#include "brake_pedal.h"
#include "cruise_control.h"
#include "front_controller.h"
#include "front_controller_hw_defs.h"
#include "front_controller_state_manager.h"
#include "front_lights_signal.h"
#include "motor_can.h"
#include "opd.h"
#include "pedal_calib_reader.h"
#include "power_manager.h"
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
  .tx = GPIO_FRONT_CONTROLLER_CAN_TX,
  .rx = GPIO_FRONT_CONTROLLER_CAN_RX,
  .loopback = false,
  .can_rx_all_cb = NULL,
};

static GpioAddress s_front_controller_board_led = GPIO_FRONT_CONTROLLER_BOARD_LED;

StatusCode front_controller_init(FrontControllerStorage *storage, FrontControllerConfig *config) {
  if (storage == NULL || config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  front_controller_storage = storage;
  front_controller_storage->config = config;

  /* Initialize hardware peripherals */
  can_init(&s_can_storage, &s_can_settings);
  // flash_init();
  // opamp_init();
  // dac_init();

  // /* Initialize front controller systems */
  // // pedal_calib_read(front_controller_storage);
  // power_manager_init(front_controller_storage);
  // brake_pedal_init(front_controller_storage);
  // accel_pedal_init(front_controller_storage);
  // opd_init(front_controller_storage);
  // ws22_motor_can_init(front_controller_storage);
  // motor_can_init(front_controller_storage);
  // cruise_control_init(front_controller_storage);
  // front_lights_signal_init();

  // front_controller_state_manager_init(front_controller_storage);

  // /* Enable Board LED */
  gpio_init_pin(&s_front_controller_board_led, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  // /* ADC initialization must happen at the very end, so all channels are registered */
  // adc_init();
  LOG_DEBUG("Front controller initialized\r\n");

  return STATUS_CODE_OK;
}
