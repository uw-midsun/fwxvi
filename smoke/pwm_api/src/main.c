/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for pwm_api
 *
 * @date   2025-03-01
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "pwm.h"
#include "status.h"
#include "tasks.h"

/* Intra-component Headers */

#define TEST_PWM_PERIOD_US 1000U

GpioAddress pwm_test_pin = { .port = GPIO_PORT_A, .pin = 5U };
uint32_t pwm_test_duty_cycle = 0U;
bool increasing_duty_cycle = true;

TASK(pwm_api, TASK_STACK_1024) {
  gpio_init_pin_af(&pwm_test_pin, GPIO_ALTFN_PUSH_PULL, GPIO_ALT1_TIM2);
  pwm_init(PWM_TIMER_2, TEST_PWM_PERIOD_US);

  while (true) {
    pwm_set_dc(PWM_TIMER_2, pwm_test_duty_cycle, PWM_CHANNEL_1, false);
    LOG_DEBUG("Test DC: %lu, Received DC: %u\n", pwm_test_duty_cycle, pwm_get_dc(PWM_TIMER_2, PWM_CHANNEL_1));

    if (increasing_duty_cycle) {
      if (pwm_test_duty_cycle < 95U) {
        pwm_test_duty_cycle++;
      } else {
        increasing_duty_cycle = false; /* We have reached the maximum of 95% */
      }
    } else {
      if (pwm_test_duty_cycle) {
        if (pwm_test_duty_cycle > 5U) {
          pwm_test_duty_cycle--;
        } else {
          increasing_duty_cycle = true; /* We have reached the minimum of 5% */
        }
      }
    }

    delay_ms(10);
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

  tasks_init_task(pwm_api, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
