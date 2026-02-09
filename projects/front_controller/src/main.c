/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for front_controller
 *
 * @date   2025-07-19
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "adc.h"
#include "can.h"
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "accel_pedal.h"
#include "brake_pedal.h"
#include "front_controller.h"
#include "front_controller_getters.h"
#include "front_controller_setters.h"
#include "front_controller_state_manager.h"
#include "motor_can.h"
#include "opd.h"
#include "power_manager.h"
#include "front_controller_hw_defs.h"
#include "ws22_motor_can.h"

FrontControllerStorage front_controller_storage = { 0 };

FrontControllerConfig front_controller_config = { .accel_input_deadzone = FRONT_CONTROLLER_ACCEL_INPUT_DEADZONE,
                                                  .accel_input_remap_min = FRONT_CONTROLLER_ACCEL_REMAP_MIN,
                                                  .accel_input_curve_exponent = FRONT_CONTROLLER_ACCEL_CURVE_EXPONENT,
                                                  .accel_low_pass_filter_alpha = FRONT_CONTROLLER_ACCEL_LPF_ALPHA,
                                                  .brake_pedal_deadzone = FRONT_CONTROLLER_BRAKE_INPUT_DEADZONE,
                                                  .brake_low_pass_filter_alpha = FRONT_CONTROLLER_BRAKE_LPF_ALPHA };

VehicleDriveState drive_state;
static GpioAddress s_front_controller_board_led = GPIO_FRONT_CONTROLLER_BOARD_LED;
void pre_loop_init() {}

void run_1000hz_cycle() {
  run_can_rx_all();

  // adc_run();
  // accel_pedal_run();
  // brake_pedal_run();
  // opd_run();
  // motor_can_update_target_current_velocity();

  // ws22_motor_can_transmit_drive_command();
}

void run_10hz_cycle() {
  run_can_tx_medium();
  // front_controller_update_state_manager_medium_cycle();
  printf("DSS: %d | LS: %d\r\n", get_steering_buttons_drive_state(), get_steering_buttons_lights());
  set_motor_velocity_motor_velocity(20);
  set_motor_velocity_vehicle_velocity(10);

  // printf("CURRENT STATE: %s\r\n", motor_can_get_current_state_str());
  // printf("ACCEL PEDAL: %ld, BRAKE PEDAL: %d\r\n", (int32_t)(front_controller_storage.accel_pedal_storage->accel_percentage * 100), front_controller_storage.brake_enabled);
}

void run_1hz_cycle() {
  run_can_tx_slow();
  // gpio_toggle_state(&s_front_controller_board_led);
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main() {
#endif
  mcu_init();
  tasks_init();
  log_init();

  front_controller_init(&front_controller_storage, &front_controller_config);

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
