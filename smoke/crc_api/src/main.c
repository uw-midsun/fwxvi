/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for crc_api
 *
 * @date   2025-02-05
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "status.h"
#include "delay.h"
#include "crc.h"

/* Intra-component Headers */
//calculator: https://crccalc.com/?crc=1234566123&method=CRC-32/ISO-HDLC&datatype=ascii&outtype=hex
TASK(crc_api, TASK_STACK_1024) {
  crc_init(CRC_LENGTH_32);
  uint32_t arr []= {1234567};
  LOG_DEBUG("Calculation: %ld\n", crc_calculate_32(arr, sizeof(arr)));
  while (true) {} 
}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(crc_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}