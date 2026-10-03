/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for hydrogen_sensor
 *
 * @date   2026-10-03
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
#include "tcixtma1.h"

TCIXTMA1Storage storage;

/* Intra-component Headers */

TASK(hydrogen_sensor, TASK_STACK_1024) {
  
  tcixtma1_init(&storage, I2C_PORT_2, 0x2E);
  float conc;

  while (true) {
    tcixtma1_get_hydrogen_concentration(&storage, &conc);
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

  tasks_init_task(hydrogen_sensor, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
