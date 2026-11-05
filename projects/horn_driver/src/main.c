/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for horn_driver
 *
 * @date   2026-10-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "mcu.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "master_tasks.h"
#include "pwm.h"

/* Intra-component Headers */
#include "horn_driver.h"

#define TIM1_CH1 8 //PA8 Q1
#define TIM1_CH1N 7 //PA7 Q3
#define TIM1_CH2 9 //PA9 Q2
#define TIM1_CH2N 0 //PB0 Q4
#define period 250 //4kHz
#define duty_cycle 0.5 //50% duty cycle

bool deadtime = false;
GpioAddress pa8 = {
  .pin = TIM1_CH1,
  .port = GPIO_PORT_A,
};
GpioAddress pa7 = {
  .pin = TIM1_CH1N,
  .port = GPIO_PORT_A,
};
GpioAddress pa9 = {
  .pin = TIM1_CH2,
  .port = GPIO_PORT_A,
};
GpioAddress pb0 = {
  .pin = TIM1_CH2N,
  .port = GPIO_PORT_B,
};

void pre_loop_init() {}

void run_1000hz_cycle() {}

void run_10hz_cycle() {}

void run_1hz_cycle() {}

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

  init_master_tasks();

  tasks_start();

  gpio_init_pin(&pa8, GPIO_MODE_LOW);
  gpio_init_pin(&pa7, GPIO_MODE_LOW);
  gpio_init_pin(&pa9, GPIO_MODE_LOW);
  gpio_init_pin(&pb0, GPIO_MODE_LOW);

  pwm_init(PWM_TIMER_1,PERIOD);
  pwm_set_dc(PWM_TIMER_1, duty_cycle * period, PWM_CHANNEL_1, true);
  LOG_DEBUG("exiting main?");
  return 0;
}

enum MOSFET_STATE {
  ALL_OFF = 0,
  Q1_Q4_ON = 1,
  Q2_Q3_ON = 2,
}

void set_mosfet_state(enum MOSFET_STATE state) {
  switch (state) {
    case ALL_OFF:
      gpio_set(&pa8, GPIO_LOW); // Q1 Off
      gpio_set(&pa7, GPIO_LOW); // Q3 Off
      gpio_set(&pa9, GPIO_LOW); // Q2 Off
      gpio_set(&pb0, GPIO_LOW); // Q4 Off
      break;
    case Q1_Q4_ON:
      gpio_set(&pa8, GPIO_HIGH); // Q1 On
      gpio_set(&pa7, GPIO_LOW); // Q3 Off
      gpio_set(&pa9, GPIO_LOW); // Q2 Off
      gpio_set(&pb0, GPIO_HIGH); // Q4 On
      break;
    case Q2_Q3_ON:
      gpio_set(&pa8, GPIO_LOW); // Q1 Off
      gpio_set(&pa7, GPIO_HIGH); // Q3 On
      gpio_set(&pa9, GPIO_HIGH); // Q2 On
      gpio_set(&pb0, GPIO_LOW); // Q4 Off
      break;
    default:
      gpio_set(&pa8, GPIO_LOW);
      gpio_set(&pa7, GPIO_LOW);
      gpio_set(&pa9, GPIO_LOW);
      gpio_set(&pb0, GPIO_LOW);
      break;
  }
  // state 1 is duty cycled
  void pwm_duty_cycle(*double duty_cycle) {
    if (deadtime) {
      set_mosfet_state(ALL_OFF);
      return;
    }
    if (pwm_get_dc(PWM_TIMER_1, PWM_CHANNEL_1)) { // wrong method
      set_mosfet_state(Q1_Q4_ON);
    } else {
      set_mosfet_state(Q2_Q3_ON);
    }
  }
}