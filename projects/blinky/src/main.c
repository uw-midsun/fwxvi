/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for Blinky
 *
 * @date   2024-11-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "tasks.h"
#include "delay.h"

/* Intra-component Headers */

GpioAddress led_green = {
  .pin  = 0U,
  .port = GPIO_PORT_A
};

TASK(Blinky, TASK_STACK_512) {
  gpio_init();
  gpio_init_pin(&led_green, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  while (true) {
    LOG_DEBUG("BLINKY\n");
    gpio_toggle_state(&led_green);
    delay_ms(500);
  }
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(Blinky, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
