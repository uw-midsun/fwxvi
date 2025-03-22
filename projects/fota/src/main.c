/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for fota
 *
 * @date   2025-03-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

/* Intra-component Headers */
#include "fota.h"

int main() {
  mcu_init();

  while (true) {
  }

  return 0;
}
