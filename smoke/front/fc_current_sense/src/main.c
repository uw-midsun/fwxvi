/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for front_controller_current_sense
 *
 * @date   2026-01-06
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

#define TBL_WIDTH 5

FrontControllerStorage front_controller_storage = { 0 };

FrontControllerConfig front_controller_config = { .accel_input_deadzone = FRONT_CONTROLLER_ACCEL_INPUT_DEADZONE,
                                                  .accel_input_remap_min = FRONT_CONTROLLER_ACCEL_REMAP_MIN,
                                                  .accel_input_curve_exponent = FRONT_CONTROLLER_ACCEL_CURVE_EXPONENT,
                                                  .accel_low_pass_filter_alpha = FRONT_CONTROLLER_ACCEL_LPF_ALPHA };

TASK(current_sense, TASK_STACK_1024) {
  StatusCode status = STATUS_CODE_OK;

  // Step 1: Check if the front controller can be initialized
  status = front_controller_init(&front_controller_storage, &front_controller_config);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("front controller initialized\r\n");
  } else {
    LOG_DEBUG("front controller cannot be initialized\r\n");
  }
  delay_ms(500);
  power_manager_set_output_group(OUTPUT_GROUP_ALL, true);

  // Step 2: Run current sense
  while (true) {
    status = power_manager_run_current_sense(OUTPUT_GROUP_ALL);

    for (uint8_t i = 0; i < NUM_OUTPUTS; i++) {
      printf("G%02d: %3dmA | ", i, front_controller_storage.power_manager_storage->current_readings[i]);
      if ((i + 1) % TBL_WIDTH == 0) {
        printf("\r\n");
      }
    }
    delay_ms(1000);
    printf("\r\r\n\n");
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

  tasks_init_task(current_sense, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
