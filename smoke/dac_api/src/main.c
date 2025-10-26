/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for dac_api
 *
 * @date   2025-10-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "dac.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */

#define DAC_API_TEST_VOLTAGE_MV 3300

TASK(dac_api, TASK_STACK_1024) {
  dac_init();
  dac_enable_channel(DAC_CHANNEL1);
  dac_set_voltage(DAC_CHANNEL1, DAC_API_TEST_VOLTAGE_MV);

  while (true) {
  }
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

  tasks_init_task(dac_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
