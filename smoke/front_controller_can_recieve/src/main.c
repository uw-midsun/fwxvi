/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for front_controller_can_recieve
 *
 * @date   2026-01-13
 * @author Midnight Sun Team #26 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <string.h>

/* Inter-component Headers */
#include "delay.h"
#include "flash.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "persist.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */
#include "front_controller.h"
#include "front_controller_getters.h"
#include "front_controller_state_manager.h"

#define CYCLE_PERIOD_MS 2000U
#define LOG_DB_DELAY 10U

FrontControllerStorage front_controller_storage = { 0 };

FrontControllerConfig front_controller_config = { .accel_input_deadzone = FRONT_CONTROLLER_ACCEL_INPUT_DEADZONE,
                                                  .accel_input_remap_min = FRONT_CONTROLLER_ACCEL_REMAP_MIN,
                                                  .accel_input_curve_exponent = FRONT_CONTROLLER_ACCEL_CURVE_EXPONENT,
                                                  .accel_low_pass_filter_alpha = FRONT_CONTROLLER_ACCEL_LPF_ALPHA };

static const char *print_state_str(FrontControllerState state) {
  switch (state) {
    case FRONT_CONTROLLER_STATE_IDLE:
      return "IDLE";
    case FRONT_CONTROLLER_STATE_ENGAGED:
      return "ENGAGED";
    case FRONT_CONTROLLER_STATE_FAULT:
      return "FAULT";
    default:
      return "UNKNOWN";
  }
}

// Printed list of relay states
static void log_status(void) {
  FrontControllerState current_state = front_controller_state_manager_get_state();
  LOG_DEBUG("State: %s\n", print_state_str(current_state));
  delay_ms(20U);
}

TASK(front_controller_run_med_cycle, TASK_STACK_1024) {
  LOG_DEBUG("Initializing the front controller...\n");
  StatusCode status = front_controller_init(&front_controller_storage, &front_controller_config);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("front controller initialized\n");
  } else {
    LOG_DEBUG("front controller cannot be initialized\n");
  }
  delay_ms(500U);

  while (true) {
    front_controller_update_state_manager_medium_cycle();
    delay_ms(100U);
  }
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char **argv) {
  mpxe_init(argc, argv);
#else
int main() {
#endif
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(front_controller_run_med_cycle, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main");
  return 0;
}
