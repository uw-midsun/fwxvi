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
#include "front_controller.h"
#include "motor_can.h"
#include "opd.h"
#include "power_manager.h"
#include "ws22_motor_can.h"

FrontControllerStorage front_controller_storage = { 0 };

FrontControllerConfig front_controller_config = { .accel_input_deadzone = FRONT_CONTROLLER_ACCEL_INPUT_DEADZONE,
                                                  .accel_input_remap_min = FRONT_CONTROLLER_ACCEL_REMAP_MIN,
                                                  .accel_input_curve_exponent = FRONT_CONTROLLER_ACCEL_CURVE_EXPONENT,
                                                  .accel_low_pass_filter_alpha = FRONT_CONTROLLER_ACCEL_LPF_ALPHA };

void pre_loop_init() {}

void run_1000hz_cycle() {
  run_can_rx_all();

  adc_run();
  accel_pedal_run();
  opd_run();
  motor_can_update_target_current_velocity();

  run_can_tx_fast();
  ws22_motor_can_transmit_drive_command();
}

void run_10hz_cycle() {
  run_can_tx_medium();
}

void run_1hz_cycle() {
  run_can_tx_slow();
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
