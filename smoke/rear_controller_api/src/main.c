/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for rear_controller_api
 *
 * @date   2025-09-15
 * @author Midnight Sun Team #24 - MSXVI
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
#include "rear_controller.h"
#include "rear_controller_state_manager.h"
#include "relays.h"

static RearControllerStorage s_rear_storage;
static RearControllerConfig s_rear_config = { 0 };

static const char *print_state_str(RearControllerState state) {
  switch (state) {
    case REAR_CONTROLLER_STATE_IDLE:
      return "IDLE";
    case REAR_CONTROLLER_STATE_DRIVE:
      return "DRIVE";
    case REAR_CONTROLLER_STATE_CHARGE:
      return "CHARGE";
    case REAR_CONTROLLER_STATE_FAULT:
      return "FAULT";
    default:
      return "UNKNOWN";
  }
}

// Printed list of relay states
static void log_status(void) {
  RearControllerState current_state = rear_controller_state_manager_get_state();

  LOG_DEBUG("State: %s | Relays: [POS: %d] [NEG: %d] [SOLAR: %d] [MOTOR: %d]\n", print_state_str(current_state), s_rear_storage.pos_relay_closed, s_rear_storage.neg_relay_closed,
            s_rear_storage.solar_relay_closed, s_rear_storage.motor_relay_closed);
}

TASK(rear_controller_smoke, TASK_STACK_1024) {
  LOG_DEBUG("Initialziing the rear controller...\n");
  s_rear_storage.config = &s_rear_config;
  s_rear_storage.precharge_complete = true;
  rear_controller_init(&s_rear_storage, &s_rear_config);
  delay_ms(100);

  while (true) {
    LOG_DEBUG("\n--- REAR CONTROLLER SMOKE TEST ---\n");
    LOG_DEBUG(">> Step 1: Simulating Initialization Complete... \n");
    log_status();
    delay_ms(1000);

    LOG_DEBUG(">> Step 2 : Requesting Drive...\n");
    rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_DRIVE_REQUEST);
    log_status();
    delay_ms(1000);

    LOG_DEBUG(">> Step 3: Requesting Neutral...\n");
    rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_NEUTRAL_REQUEST);
    log_status();
    delay_ms(1000);

    LOG_DEBUG(">> Step 4: Requesting Charge...\n");
    rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_CHARGE_REQUEST);
    log_status();
    delay_ms(1000);

    LOG_DEBUG(">> Step 5: Removing Charger...\n");
    rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_CHARGER_REMOVED);
    log_status();
    delay_ms(1000);

    LOG_DEBUG(">> Step 6: Simulating Fault...\n");
    rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_FAULT);
    log_status();
    delay_ms(1000);

    LOG_DEBUG(">> Step 7: Resetting from Fault...\n");
    rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_RESET);
    log_status();
    delay_ms(1000);
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

  tasks_init_task(rear_controller_smoke, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main");
  return 0;
}
