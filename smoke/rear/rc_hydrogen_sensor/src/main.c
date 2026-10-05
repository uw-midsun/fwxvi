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
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"
#include "tcixtma1.h"

TCIXTMA1Storage storage;

/* Intra-component Headers */

TASK(hydrogen_sensor, TASK_STACK_1024) {
  
  // TCI I2C address is 0x36 (datasheet section 10.1.1)
  tcixtma1_init(&storage, I2C_PORT_2, 0x36);
  float conc = 0.0f;

  while (true) {
    StatusCode status = tcixtma1_get_hydrogen_concentration(&storage, &conc);

    /* conc is in vol%, smallest sensor unit is 0.01 vol% = 100 ppm (datasheet section 10.2)*/
    if (status == STATUS_CODE_OK){
      LOG_DEBUG("Hydrogen concentration: %.2f (vol%%)\n", conc);
    } else {
      LOG_DEBUG("Read failed\n");
    }

    vTaskDelay(100U);
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
