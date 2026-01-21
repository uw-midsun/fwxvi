/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for front_controller_pedal
 *
 * @date   2026-01-21
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

#include "accel_pedal.h"
#include "brake_pedal.h"

FrontControllerStorage front_controller_storage = { 0 };

FrontControllerConfig front_controller_config = { .accel_input_deadzone = FRONT_CONTROLLER_ACCEL_INPUT_DEADZONE,
                                                  .accel_input_remap_min = FRONT_CONTROLLER_ACCEL_REMAP_MIN,
                                                  .accel_input_curve_exponent = FRONT_CONTROLLER_ACCEL_CURVE_EXPONENT,
                                                  .accel_low_pass_filter_alpha = FRONT_CONTROLLER_ACCEL_LPF_ALPHA,
                                                  .brake_pedal_deadzone = FRONT_CONTROLLER_BRAKE_INPUT_DEADZONE,
                                                  .brake_low_pass_filter_alpha = FRONT_CONTROLLER_BRAKE_LPF_ALPHA };

TASK(run_pedal, TASK_STACK_1024) {
  StatusCode status = STATUS_CODE_OK;

  // Step 1: Check if the front controller can be initialized
  status = front_controller_init(&front_controller_storage, &front_controller_config);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("front controller initialized\r\n");
  } else {
    LOG_DEBUG("front controller cannot be initialized\r\n");
  }
  delay_ms(500);

  // Step 2: Run pedals
  while (true) {
    adc_run();
    accel_pedal_run();
    brake_pedal_run();
  }
}

TASK(display_pedal_stats, TASK_STACK_1024) {
  delay_ms(1000U);

  while (true) {
    printf("ACCEL PEDAL: %ld, BRAKE PEDAL: %d\r\n", (int32_t)(front_controller_storage.accel_pedal_storage->accel_percentage), front_controller_storage.brake_enabled);
    delay_ms(100U);
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

  tasks_init_task(run_pedal, TASK_PRIORITY(3), NULL);
  tasks_init_task(display_pedal_stats, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
