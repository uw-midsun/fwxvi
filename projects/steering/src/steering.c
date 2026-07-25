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
#include "gui_menu.h"
#include "log.h"
#include "mcu.h"
#include "persist.h"
#include "system_can.h"

/* Intra-component Headers */
#include "button_led_manager.h"
#include "button_manager.h"
#include "buzzer.h"
#include "cruise_control.h"
#include "display.h"
#include "drive_state_manager.h"
#include "light_signal_manager.h"
#include "motor_can.h"
#include "party_mode.h"
#include "pedal_calib.h"
#include "range_estimator.h"
#include "steering.h"
#include "steering_hw_defs.h"
#include "steering_setters.h"

#define LAST_PAGE 255U

/************************************************************************************************
 * Storage definitions
 ************************************************************************************************/

static SteeringStorage *steering_storage;

static ButtonManager s_button_manager = { 0 };

static CanStorage s_can_storage = { 0 };
// static PersistStorage persist_storage = { 0 };

/** @brief   Cell-balancing request broadcast to the rear controller (rear gates balancing on this) */
static bool s_cell_discharge_requested = false;

/**
 * @brief   Toggle the cell-discharge request broadcast in the steering buttons bitfield
 * @details Invoked by the "Toggle Cell Discharge" overlay menu item. The rear controller only
 *          balances when its local BALANCING_ENABLED master is set AND this request is true.
 * @return  STATUS_CODE_OK on success
 */
static StatusCode s_toggle_cell_discharge(void) {
  s_cell_discharge_requested = !s_cell_discharge_requested;
  set_steering_buttons_balancing_enabled(s_cell_discharge_requested);
  buzzer_play_success();
  return STATUS_CODE_OK;
}

/** @brief   BPS enable broadcast to the rear controller. Defaults enabled so protection is on from boot */
static bool s_bps_enabled = true;

static StatusCode s_toggle_bps(void) {
  s_bps_enabled = !s_bps_enabled;
  set_steering_buttons_bps_enabled(s_bps_enabled);
  buzzer_play_success();
  return STATUS_CODE_OK;
}

StatusCode steering_force_disable_bps(void) {
  s_bps_enabled = false;
  set_steering_buttons_bps_enabled(false);
  gui_menu_set_bps_enabled(false);
  return STATUS_CODE_OK;
}

/************************************************************************************************
 * Settings definitions
 ************************************************************************************************/

static const CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_STEERING,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = GPIO_STEERING_CAN_TX,
  .rx = GPIO_STEERING_CAN_RX,
  .loopback = false,
  .can_rx_all_cb = motor_can_process_rx,
};

StatusCode steering_init(SteeringStorage *storage, SteeringConfig *config, Ws22MotorCanConfig *motor_can_config) {
  if (storage == NULL || config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  steering_storage = storage;
  steering_storage->config = config;

  // persist_init(&persist_storage, LAST_PAGE, &(steering_storage->persist_data), sizeof(steering_storage->persist_data), false);
  // steering_storage->persist_storage = &persist_storage;

  can_init(&s_can_storage, &s_can_settings);
  ws22_motor_can_init(storage->ws22_motor_can_storage, motor_can_config);
  lights_signal_manager_init(steering_storage);
  button_led_manager_init(steering_storage);
  button_manager_init(steering_storage);
  buzzer_init();
  display_init(steering_storage);
  party_mode_init(steering_storage);
  gui_menu_set_party_mode_callback(party_mode_toggle);
  gui_menu_set_toggle_discharge_callback(s_toggle_cell_discharge);
  gui_menu_set_toggle_bps_callback(s_toggle_bps);
  set_steering_buttons_bps_enabled(s_bps_enabled);
  cruise_control_init(steering_storage);
  range_estimator_init(steering_storage);
  drive_state_manager_init(steering_storage);
  steering_pedal_calib_init(steering_storage);

  buzzer_play_startup();

  steering_force_disable_bps();

  // button_led_manager_clear_all();
  return STATUS_CODE_OK;
}
