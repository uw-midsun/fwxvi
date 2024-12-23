/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for blinky
 *
 * @date   2024-11-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "blinky.h"

GpioAddress pa0_led = { .pin = 0U, .port = GPIO_PORT_A };

TASK(blinky, TASK_STACK_512) {
  gpio_init_pin(&pa0_led, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  while (true) {
    LOG_DEBUG("Blinky!\n");
    gpio_toggle_state(&pa0_led);
    delay_ms(500);
  }
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(blinky, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
