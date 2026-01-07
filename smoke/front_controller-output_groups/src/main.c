/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for front_controller - output_groups
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

#define OUTPUT_GRP_TO_STR(x)                            \
  ((x) == OUTPUT_GROUP_ALL      ? "OUTPUT_GROUP_ALL"    \
   : (x) == IDLE_GROUP          ? "IDLE_GROUP"          \
   : (x) == LEFT_LIGHTS_GROUP   ? "LEFT_LIGHTS_GROUP"   \
   : (x) == RIGHT_LIGHTS_GROUP  ? "RIGHT_LIGHTS_GROUP"  \
   : (x) == HAZARD_LIGHTS_GROUP ? "HAZARD_LIGHTS_GROUP" \
   : (x) == BPS_LIGHTS_GROUP    ? "BPS_LIGHTS_GROUP"    \
   : (x) == BRAKE_LIGHTS_GROUP  ? "BRAKE_LIGHTS_GROUP"  \
   : (x) == HORN_GROUP          ? "HORN_GROUP"          \
                                : "UNKNOWN")

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
    LOG_DEBUG("front controller initialized\n");
  } else {
    LOG_DEBUG("front controller cannot be initialized\n");
  }
  delay_ms(500);

  // Step 2: Cycle through valid output groups

  uint8_t num_output_group = OUTPUT_GROUP_ALL;
  bool state = false;

  while (true) {
    power_control_set_output_group(num_output_group, state);
    printf("Setting output group %s to state %d", OUTPUT_GRP_TO_STR(num_output_group), state);

    if (state == true) {
      state = false;
    } else {
      num_output_group++;
      if (num_output_group >= NUM_OUTPUT_GROUPS) {
        num_output_group %= NUM_OUTPUT_GROUPS;
      }
      state = true;
    }

    delay_ms(100);
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
