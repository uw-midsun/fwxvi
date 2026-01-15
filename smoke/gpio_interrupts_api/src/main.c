/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for gpio_interrupts_api
 *
 * @date   2025-07-19
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"
#include "gpio_interrupts.h"

// port + pin

/* Intra-component Headers */

TASK(gpio_interrupts_api, TASK_STACK_1024) {
// to do
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
}
#else
int main() {
#endif
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(gpio_interrupts_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
