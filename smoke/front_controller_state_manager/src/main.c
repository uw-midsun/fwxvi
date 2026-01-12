/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for front_controller_state_manager
 *
 * @date   2026-01-12
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

TASK(front_controller_simulate_can, TASK_STACK_1024) {
  LOG_DEBUG("Initializing the CAN simulation task...\n");
  delay_ms(10U);

  while (true) {
    LOG_DEBUG("\n--- FRONT CONTROLLER SMOKE TEST ---\n");
    g_rx_struct.rear_controller_status_bps_fault = 0;
    g_rx_struct.battery_stats_B_motor_precharge_complete = 0;
    g_rx_struct.steering_buttons_drive_state = 0;
    g_rx_struct.steering_buttons_horn_enabled = 0;
    g_rx_struct.steering_buttons_lights = 0;
    delay_ms(LOG_DB_DELAY);
    LOG_DEBUG(">> Step 1: Simulating Initialization Complete... \n");
    delay_ms(LOG_DB_DELAY);
    log_status();
    delay_ms(CYCLE_PERIOD_MS);

    LOG_DEBUG(">> Step 2 : Requesting Drive With Incomplete Precharge...\n");
    g_rx_struct.steering_buttons_drive_state = VEHICLE_DRIVE_STATE_DRIVE;
    g_rx_struct.battery_stats_B_motor_precharge_complete = 0U;
    log_status();
    delay_ms(CYCLE_PERIOD_MS);

    LOG_DEBUG(">> Step 3 : Requesting Drive With Complete Precharge...\n");
    g_rx_struct.steering_buttons_drive_state = VEHICLE_DRIVE_STATE_DRIVE;
    g_rx_struct.battery_stats_B_motor_precharge_complete = 1U;
    log_status();
    delay_ms(CYCLE_PERIOD_MS);

    LOG_DEBUG(">> Step 4 : Steering Lights Left...\n");
    g_rx_struct.steering_buttons_lights = STEERING_LIGHTS_LEFT_STATE;
    log_status();
    delay_ms(CYCLE_PERIOD_MS);

    LOG_DEBUG(">> Step 5 : Steering Lights Right...\n");
    g_rx_struct.steering_buttons_lights = STEERING_LIGHTS_RIGHT_STATE;
    log_status();
    delay_ms(CYCLE_PERIOD_MS);

    LOG_DEBUG(">> Step 6 : Steering Lights Hazard...\n");
    g_rx_struct.steering_buttons_lights = STEERING_LIGHTS_HAZARD_STATE;
    log_status();
    delay_ms(CYCLE_PERIOD_MS);

    LOG_DEBUG(">> Step 7 : Steering Lights Off...\n");
    g_rx_struct.steering_buttons_lights = STEERING_LIGHTS_OFF_STATE;
    log_status();
    delay_ms(CYCLE_PERIOD_MS);

    LOG_DEBUG(">> Step 8 : Enabling Horn...\n");
    g_rx_struct.steering_buttons_horn_enabled = 1U;
    log_status();
    delay_ms(CYCLE_PERIOD_MS);

    LOG_DEBUG(">> Step 9 : Disabling Horn...\n");
    g_rx_struct.steering_buttons_horn_enabled = 0U;
    log_status();
    delay_ms(CYCLE_PERIOD_MS);

    LOG_DEBUG(">> Step 10 : Simulating BPS fault...\n");
    g_rx_struct.rear_controller_status_bps_fault = 1U;
    log_status();
    delay_ms(CYCLE_PERIOD_MS);

    LOG_DEBUG(">> Step 11 : Exiting BPS fault...\n");
    g_rx_struct.rear_controller_status_bps_fault = 0U;
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

  tasks_init_task(front_controller_run_med_cycle, TASK_PRIORITY(3), NULL);
  tasks_init_task(front_controller_simulate_can, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main");
  return 0;
}
