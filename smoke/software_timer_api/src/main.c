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
#include "delay.h"
#include "log.h"
#include "mpxe.h"
#include "tasks.h"

/* Intra-component Headers */
#include "software_timer.h"

static SoftTimer s_timer;
static SoftwareWatchdog s_watchdog;

// Simple callback functions to verify timers work

// Soft timer callback
static void timer_callback(SoftTimerId id) {
  LOG_DEBUG("Soft Timer callback triggered!\n");
}

// Watchdog fault callback
static void watchdog_fault_callback(SoftTimerId id) {
  LOG_DEBUG("WATCHDOG FAULT triggered!\n");
}

TASK(software_timer_api, TASK_STACK_1024) {
  LOG_DEBUG("-------- WATCHDOG SMOKE TEST START --------\n");

  // 1️⃣ Test soft timer normally
  LOG_DEBUG("Starting soft timer (1000 ms)\n");
  software_timer_init_and_start(1000, timer_callback, &s_timer);

  delay_ms(500);
  LOG_DEBUG("Soft timer in use? %d\n", software_timer_inuse(&s_timer));

  delay_ms(600);  // let the timer expire
  LOG_DEBUG("Soft timer in use after expiry? %d\n", software_timer_inuse(&s_timer));

  // 2️⃣ Test watchdog timer
  LOG_DEBUG("Initializing watchdog (2000 ms period)\n");
  software_watchdog_init(&s_watchdog, 2000, watchdog_fault_callback);

  LOG_DEBUG("Kicking watchdog after 1 second\n");
  delay_ms(1000);
  software_watchdog_kick(&s_watchdog);  // reset timer before it expires

  LOG_DEBUG("Kicking watchdog again after 1 second\n");
  delay_ms(1000);
  software_watchdog_kick(&s_watchdog);  // reset timer again

  LOG_DEBUG("Now NOT kicking watchdog and letting it expire\n");
  delay_ms(2500);  // exceed watchdog period → should trigger fault

  LOG_DEBUG("-------- WATCHDOG SMOKE TEST COMPLETE --------\n");

  while (true) {
    delay_ms(1000);
  }
}

#ifdef MS_PLATFORM_X86
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main(void) {
#endif
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(software_timer_api, TASK_PRIORITY(3), NULL);
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
