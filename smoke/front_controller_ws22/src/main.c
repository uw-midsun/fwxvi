/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for front_controller_ws22
 *
 * @date   2026-01-18
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "front_controller.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "power_manager.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */

FrontControllerStorage front_controller_storage = { 0 };

FrontControllerConfig front_controller_config = { .accel_input_deadzone = FRONT_CONTROLLER_ACCEL_INPUT_DEADZONE,
                                                  .accel_input_remap_min = FRONT_CONTROLLER_ACCEL_REMAP_MIN,
                                                  .accel_input_curve_exponent = FRONT_CONTROLLER_ACCEL_CURVE_EXPONENT,
                                                  .accel_low_pass_filter_alpha = FRONT_CONTROLLER_ACCEL_LPF_ALPHA };

TASK(main_cycle, TASK_STACK_1024) {
  StatusCode status = STATUS_CODE_OK;

  // Step 1: Check if the front controller can be initialized
  status = front_controller_init(&front_controller_storage, &front_controller_config);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("front controller initialized\n");
  } else {
    LOG_DEBUG("front controller cannot be initialized\n");
  }
  delay_ms(500);

  while (true) {
    run_can_rx_all();
    adc_run();
    accel_pedal_run();
    opd_run();
    motor_can_update_target_current_velocity();
    run_can_tx_fast();
    ws22_motor_can_transmit_drive_command();
    delay_ms(1U);
  }
}

TASK(cycle_ws22, TASK_STACK_1024) {
  delay_ms(2000U);

  LOG_DEBUG("Starting to mock pedal values");

  OutputGroup num_output_group = OUTPUT_GROUP_ALL;
  bool state = false;

  while (true) {
    delay_ms(500);
  }
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

  tasks_init_task(cycle_ws22, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
