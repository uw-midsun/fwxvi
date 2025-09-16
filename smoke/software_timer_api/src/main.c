/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for software_timer_api
 *
 * @date   2025-09-15
 * @author Arthigan Jeyamohan
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "status.h"
#include "delay.h"
#include "log.h"

/* Functions to test
soft_timer_init_and_start
soft_timer_init
soft_timer_start
soft_timer_cancel
soft_timer_reset
soft_timer_inuse
soft_timer_remaining_time
*/

/* Intra-component Headers */


TASK(software_timer_api, TASK_STACK_1024) {

  StatusCode status = STATUS_CODE_OK;

  while (true) {
    LOG_DEBUG("Running Software Timer API Smoke!\n");

    do {

      //Setting up time to call callback function

      uint32_t time = 500;


    } while(false);

    LOG_DEBUG("Done test! Exited with status code %d\n", status);
    status = STATUS_CODE_OK;
    delay_ms(500);
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

  tasks_init_task(software_timer_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}