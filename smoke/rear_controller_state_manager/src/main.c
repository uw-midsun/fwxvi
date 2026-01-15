/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for rear_controller_state_manager
 *
 * @date   2026-01-09
 * @author Midnight Sun Team #26 - MSXVI
 ************************************************************************************************/

/**
 * !Note: In order for this test to work, either the relays should be plugged in or
 *        RELAYS_RESPECT_CURRENT_SENSE may be set to 0U in relays.c
 *
 * Expected behavior from this test:
 * rear_controller_state_manager.c should cycle through the valid states in rear controller
 * following this sequence:
 *
 * 1. Initialization                            expected state: IDLE
 * 2. Drive request                             expected state: DRIVE (we must mock precharge_complete = true)
 * 3. Neutral request                           expected state: IDLE
 * 4. Drive request with incomplete precharge   expected state: IDLE (we must mock precharge_complete = false)
 * 4. Simulate a fault                          expected state: FAULT
 * 5. Reset from fault                          expect state, IDLE
 *
 * The expected LED sequence in each state:
 *
 * IDLE:  solar: 1, pos: 1, neg: 1, motor: 0, ws22: 0
 * DRIVE: solar: 1, pos: 1, neg: 1, motor: 1, ws22: 1
 * FAULT: solar: 0, pos: 0, neg: 0, motor: 0, ws22: 0
 */

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

#define CYCLE_PERIOD_MS 1000U
#define LOG_DB_DELAY 10U

static RearControllerStorage s_rear_storage;
static RearControllerConfig s_rear_config = { 0 };

static const char *print_state_str(RearControllerState state) {
  switch (state) {
    case REAR_CONTROLLER_STATE_IDLE:
      return "IDLE";
    case REAR_CONTROLLER_STATE_DRIVE:
      return "DRIVE";
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
  delay_ms(20U);
}

TASK(rear_controller_smoke, TASK_STACK_1024) {
  LOG_DEBUG("Initializing the rear controller...\n");
  s_rear_storage.config = &s_rear_config;
  rear_controller_init(&s_rear_storage, &s_rear_config);
  delay_ms(100U);

  while (true) {
    LOG_DEBUG("\n--- REAR CONTROLLER SMOKE TEST ---\n");
    delay_ms(LOG_DB_DELAY);
    LOG_DEBUG(">> Step 1: Simulating Initialization Complete... \n");
    delay_ms(LOG_DB_DELAY);
    log_status();
    delay_ms(CYCLE_PERIOD_MS);

    LOG_DEBUG(">> Step 2 : Requesting Drive...\n");
    s_rear_storage.precharge_complete = true;
    delay_ms(LOG_DB_DELAY);
    rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_DRIVE_REQUEST);
    log_status();
    delay_ms(CYCLE_PERIOD_MS);

    LOG_DEBUG(">> Step 3: Requesting Neutral...\n");
    delay_ms(LOG_DB_DELAY);
    rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_NEUTRAL_REQUEST);
    log_status();
    delay_ms(CYCLE_PERIOD_MS);

    LOG_DEBUG(">> Step 4 : Requesting Drive with Incomplete Precharge...\n");
    s_rear_storage.precharge_complete = false;
    delay_ms(LOG_DB_DELAY);
    rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_DRIVE_REQUEST);
    log_status();
    delay_ms(CYCLE_PERIOD_MS);

    LOG_DEBUG(">> Step 5: Simulating Fault...\n");
    delay_ms(LOG_DB_DELAY);
    rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_FAULT);
    log_status();
    delay_ms(CYCLE_PERIOD_MS);

    LOG_DEBUG(">> Step 6: Resetting from Fault...\n");
    delay_ms(LOG_DB_DELAY);
    rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_RESET);
    log_status();
    delay_ms(CYCLE_PERIOD_MS);
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
