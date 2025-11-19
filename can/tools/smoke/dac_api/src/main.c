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
#include "dac.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */

#define DAC_API_TEST_VOLTAGE_MV 3300

TASK(dac_api, TASK_STACK_1024) {
  StatusCode status = STATUS_CODE_OK;

  // Test 1: Checking if the DAC can initialize
  status = dac_init();
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("DAC initialized\n");
  } else {
    LOG_DEBUG("DAC cannot be initialized\n");
  }
  delay_ms(500);

  // Test 2: Enable channel 1
  status = dac_enable_channel(DAC_CHANNEL1);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("Channel 1 enabled\n");
  } else {
    LOG_DEBUG("Channel 1 cannot be enabled\n");
  }
  delay_ms(500);

  // Test 3: Set the voltage
  status = dac_set_voltage(DAC_CHANNEL1, DAC_API_TEST_VOLTAGE_MV);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("Voltage set, should read 3300mV\n");
  } else {
    LOG_DEBUG("Voltage cannot be set\n");
  }
  delay_ms(500);

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
