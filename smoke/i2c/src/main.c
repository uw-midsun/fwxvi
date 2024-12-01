/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for [PROJECT NAME]
 *
 * @date   [YYYY/MM/DD]
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "i2c.h"
#include "log.h"
#include "tasks.h"
#include "master_tasks.h"

/* Intra-component Headers */

GpioAddress pa0_led = {
  .pin  = 0U,
  .port = GPIO_PORT_A
};

TASK(Blinky, TASK_STACK_512) {
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

  tasks_init_task(Blinky, TASK_PRIORITY(3UL), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
