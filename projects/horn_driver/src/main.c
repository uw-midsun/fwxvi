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
#include "stm32l4xx_hal_tim.h"

/* Intra-component Headers */
#include "horn_driver.h"

#define TIM1_CH1 8 //PA8 Q1
#define TIM1_CH1N 7 //PA7 Q3
#define TIM1_CH2 9 //PA9 Q2
#define TIM1_CH2N 0 //PB0 Q4
#define period 250 //4kHz
#define duty_cycle 0.5 //50% duty cycle
#define dead_time_duty_cycle 0.04

static bool value
int last_value = -1;
static TIM_HandleTypeDef h_timer = {0U};
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
}
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

  pwm_init(PWM_TIMER_1,period);
  pwm_set_dc(PWM_TIMER_1, duty_cycle * period, PWM_CHANNEL_1, true);

  
  HAL_TIM_Base_Init(&h_timer);
  HAL_TIM_Base_Start_IT(&h_timer);

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
}
void delay_us (uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&h_timer,0);  // set the counter value a 0
    while (__HAL_TIM_GET_COUNTER(&h_timer) < us);  // wait for the counter to reach the us input in the parameter
}
  // state 1 is duty cycled
void pwm_duty_cycle(*double duty_cycle) {
  while (true) {
    if (last_value != pwm_get_dc(PWM_TIMER_1, PWM_CHANNEL_1)) {
      last_value = pwm_get_dc(PWM_TIMER_1, PWM_CHANNEL);
      delay_us(5);
      LOG_DEBUG("delay");
      return;
    }
    if (pwm_get_dc(PWM_TIMER_1, PWM_CHANNEL_1)) {
      set_mosfet_state(Q1_Q4_ON);
      LOG_DEBUG("state Q1 Q4");
    } else {
      set_mosfet_state(Q2_Q3_ON);
      LOG_DEBUG("state Q2 Q3");
    }
    last_value = pwm_get_dc(PWM_TIMER_1, PWM_CHANNEL);
  }
}
