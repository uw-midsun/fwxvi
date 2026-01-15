/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for steering
 *
 * @date   2025-07-09
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "can.h"
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "button_led_manager.h"
#include "button_manager.h"
#include "cruise_control.h"
#include "party_mode.h"
#include "steering.h"

SteeringStorage steering_storage;

SteeringConfig steering_config = { .cruise_max_speed_kmh = STEERING_CRUISE_MAX_SPEED_KMH, .cruise_min_speed_kmh = STEERING_CRUISE_MIN_SPEED_KMH };

void pre_loop_init() {}

void run_1000hz_cycle() {
  button_manager_update();
}

void run_10hz_cycle() {
  button_led_manager_update();
  run_can_tx_medium();
}

void run_1hz_cycle() {
  cruise_control_run();
  party_mode_run();
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

  steering_init(&steering_storage, &steering_config);

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
