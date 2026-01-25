/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for display_brightness
 *
 * @date   2026-01-24
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
#include "display.h"

/* Intra-component Headers */
bool increasing_duty_cycle = true;
uint8_t percent = 0;

TASK(display_brightness, TASK_STACK_1024) {

  StatusCode ret = setup_brightness_pwm();

  if (ret == STATUS_CODE_OK) {
    while (true) {
      if (increasing_duty_cycle) {
        while (percent < 100) {
          ++percent;
          display_set_brightness(percent);
          delay_ms(10);
        }
        increasing_duty_cycle = false;
      } else {
        while (percent > 0) {
          --percent;
          display_set_brightness(percent);
          delay_ms(10);
        }
        increasing_duty_cycle = true;
      }
    }
  } else {
    LOG_DEBUG("brightness_pwm could not be initialized");
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

  tasks_init_task(display_brightness, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
