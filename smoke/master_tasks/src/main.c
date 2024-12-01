/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for master tasks smoke
 *
 * @date   2024-11-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */

TickType_t last_time_1000hz = 0U;
TickType_t last_time_10hz = 0U;
TickType_t last_time_1hz = 0U;

GpioAddress pa0_led = {
  .pin  = 0U,
  .port = GPIO_PORT_A
};

void pre_loop_init() {
  gpio_init_pin(&pa0_led, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
}

void run_1000hz_cycle() {
}

void run_10hz_cycle() {
  TickType_t time_elapsed = xTaskGetTickCount() - last_time_10hz;
  last_time_10hz = xTaskGetTickCount();
  LOG_DEBUG("Running 10hz cycle. Time elapsed between cycles: %ld\n", time_elapsed);
}

void run_1hz_cycle() {
  TickType_t time_elapsed = xTaskGetTickCount() - last_time_1hz;
  last_time_1hz = xTaskGetTickCount();
  LOG_DEBUG("Running 1hz cycle. Time elapsed between cycles: %ld\n", time_elapsed);
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
