/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for software_timer_api
 *
 * @date   2025-09-15
 * @author Arthigan Jeyamohan
 ************************************************************************************************/

/* Standard library Headers */

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
#include "soft_timer.h"
#include "delay.h"
#include "log.h"
#include "tasks.h"

static SoftTimer s_timer1;
static SoftTimer s_timer2;

// Simple callback functions to verify timers work
static void prv_timer_callback1(SoftTimerId id) {
  LOG_DEBUG("TIMER 1 callback triggered!\n");
}

static void prv_timer_callback2(SoftTimerId id) {
  LOG_DEBUG("TIMER 2 callback triggered!\n");
}

TASK(software_timer_api, TASK_STACK_1024) {
  LOG_DEBUG("-------- SOFTWARE TIMER SMOKE TEST --------\n\n");

  // Initialize and start timer 1 (1 second)
  LOG_DEBUG("Initializing and starting Timer 1 (1000 ms)\n");
  soft_timer_init_and_start(1000, prv_timer_callback1, &s_timer1);

  // Initialize and start timer 2 (2 seconds)
  LOG_DEBUG("Initializing and starting Timer 2 (2000 ms)\n");
  soft_timer_init_and_start(2000, prv_timer_callback2, &s_timer2);

  delay_ms(500);
  LOG_DEBUG("Timer 1 in use? %d\n", soft_timer_inuse(&s_timer1));
  LOG_DEBUG("Timer 2 in use? %d\n", soft_timer_inuse(&s_timer2));

  LOG_DEBUG("Remaining time for Timer 1: %lu ms\n", soft_timer_remaining_time(&s_timer1));
  LOG_DEBUG("Remaining time for Timer 2: %lu ms\n", soft_timer_remaining_time(&s_timer2));

  delay_ms(1200);
  LOG_DEBUG("After 1.2s delay -> Timer 1 in use? %d\n", soft_timer_inuse(&s_timer1));
  LOG_DEBUG("Remaining time for Timer 2: %lu ms\n", soft_timer_remaining_time(&s_timer2));

  // Reset timer 2 (should restart it)
  LOG_DEBUG("Resetting Timer 2\n");
  soft_timer_reset(&s_timer2);

  delay_ms(500);
  LOG_DEBUG("Timer 2 remaining time after reset: %lu ms\n", soft_timer_remaining_time(&s_timer2));

  // Cancel timer 2 before it expires
  LOG_DEBUG("Canceling Timer 2\n");
  soft_timer_cancel(&s_timer2);
  LOG_DEBUG("Timer 2 in use after cancel? %d\n", soft_timer_inuse(&s_timer2));

  // Reuse timer 1 (should clean up correctly)
  LOG_DEBUG("Re-initializing Timer 1 for 1500 ms\n");
  soft_timer_init_and_start(1500, prv_timer_callback1, &s_timer1);

  delay_ms(2000);
  LOG_DEBUG("Timer 1 done? %d\n", !soft_timer_inuse(&s_timer1));

  LOG_DEBUG("-------- SOFTWARE TIMER SMOKE TEST COMPLETE --------\n\n");

  while (true) {
    delay_ms(1000);
  }
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
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