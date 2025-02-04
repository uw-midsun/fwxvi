/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for pwm_api
 *
 * @date   2025-01-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "pwm.h"
#include "log.h"
#include "tasks.h"
#include "status.h"
#include "delay.h"

/* Intra-component Headers */
GpioAddress pa0_led = { .pin = 10u, .port = GPIO_PORT_B };
TASK(PwmTest, TASK_STACK_512) {
  // gpio_init_pin(&pa0_led, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  gpio_init_pin_af(&pa0_led, GPIO_STATE_LOW, GPIO_ALT1_TIM2);  // Set PA0 to PWM output
  StatusCode status = pwm_init(PWM_TIMER_2, 1000);  // Initialize PWM with 1000us period
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Failed to initialize: %d\n", status);
  }

  status = pwm_set_dc(PWM_TIMER_2, 50, PWM_CHANNEL_3, false);  // Set 50% duty cycle
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Failed to set duty cycle: %d\n", status);
  }

  status = pwm_set_pulse(PWM_TIMER_2, 500, PWM_CHANNEL_3, false);  // Set pulse width to 500us
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Failed to set pulse width: %d\n", status);
  }

 
  while(true) {
    uint16_t period = pwm_get_period(PWM_TIMER_2);  // Corrected call
    if (period != 1000) {
      LOG_DEBUG("Unexpected period: %d\n", period);
    } else {
      // gpio_init_pin(&pa0_led, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
      // gpio_toggle_state(&pa0_led);  // Toggle LED to indicate success
      LOG_DEBUG("PWM smoke test passed\n");
      delay_ms(1000);
    }
  
  }

}

int main() {
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(PwmTest, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}