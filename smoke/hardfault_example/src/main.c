/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for hardfault_example
 *
 * @date   2025-01-27
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

/* Intra-component Headers */

int illegal_instruction_execution(void) {
  int (*bad_instruction)(void) = (void *)0xE0000000U;
  return bad_instruction();
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  while (true) {
    int fault = illegal_instruction_execution();
  }

  LOG_DEBUG("exiting main?");
  return 0;
}
