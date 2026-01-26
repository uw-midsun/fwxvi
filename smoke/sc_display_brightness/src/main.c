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
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "status.h"
#include "tasks.h"
#include "pwm.h"

/* Intra-component Headers */

#define BRIGHTNESS_TIMER GPIO_ALT1_TIM2
#define BRIGHTNESS_CHANNEL PWM_CHANNEL_2

static GpioAddress s_ltdc_pwm_pin = { .port = GPIO_PORT_A, .pin = 1 }; /* Current control for brightness ctrl */

static StatusCode current_pwm_init() {
  /* configure GPIO pin */
  status_ok_or_return(gpio_init_pin_af(&s_ltdc_pwm_pin, GPIO_ALTFN_PUSH_PULL, BRIGHTNESS_TIMER));

  /* initialize PWM timer with a period */
  /* 25kHz from datasheet, 1/25000 = 40 microseconds*/
  pwm_init(BRIGHTNESS_TIMER, 10);

  /* set initial brightnes to 50% */
  pwm_set_dc(BRIGHTNESS_TIMER, 50, BRIGHTNESS_CHANNEL, false);
  return STATUS_CODE_OK;
}

static StatusCode display_set_brightness(uint8_t percentage) {
  return pwm_set_dc(BRIGHTNESS_TIMER, percentage, BRIGHTNESS_CHANNEL, false);
}

TASK(display_brightness, TASK_STACK_1024) {
  StatusCode ret = current_pwm_init();

  if (ret != STATUS_CODE_OK) {
    LOG_DEBUG("brightness_pwm could not be initialized");
    return;
  }

  bool increasing_duty_cycle = true;
  uint8_t percent = 0;

  while (true) {
    if (increasing_duty_cycle) {
      /* Slowly brigthen display */
      while (percent < 100) {
        ++percent;
        ret = display_set_brightness(percent);
        delay_ms(10);
      }
      increasing_duty_cycle = false;
    } else {
      /* Slowly Dim display */
      while (percent > 0) {
        --percent;
        ret = display_set_brightness(percent);
        delay_ms(10);
      }
      increasing_duty_cycle = true;
    }

    if (ret != STATUS_CODE_OK) {
      LOG_DEBUG("Error setting brigthness");
      return;
    }
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
