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
#define dead_time 5 // in us

enum MOSFET_STATE {
  ALL_OFF = 0,
  Q1_Q4_ON = 1,
  Q2_Q3_ON = 2,
}

MOSFET_STATE last = ALL_OFF;
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
  pwm_set_dc(PWM_TIMER_1, duty_cycle, PWM_CHANNEL_1, true); // Q1 on first, Q3 off first
  pwm_set_dc(PWM_TIMER_1, duty_cycle, PWM_CHANNEL_2,true); // Q2 on first, Q4 off first
  // need to invert second pwm since Q4 should be on first
  
  HAL_TIM_Base_Init(&h_timer);
  HAL_TIM_Base_Start_IT(&h_timer);

  dead_time();

  LOG_DEBUG("exiting main?");
  return 0;
}



MOSFET_STATE get_mosfet_state() {
  GpioState Q1_state = gpio_get_state(&pa8); // Q1
  GpioState Q3_state = gpio_get_state(&pa7) //Q3
  GpioState Q2_state = gpio_get_state(&pa9) //Q2
  GpioState Q4_state = gpio_get_state(&pb0) //Q4

  if (Q1_state == GPIO_STATE_HIGH && Q4_state == GPIO_STATE_HIGH) {
    return Q1_Q4_ON
  } else if (Q2_state == GPIO_STATE_HIGH && Q3_state = GPIO_STATE_HIGH) {
    return Q2_Q3_ON;
  }
  return ALL_OFF;
}
void delay_us (uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&h_timer,0);  // set the counter value a 0
    while (__HAL_TIM_GET_COUNTER(&h_timer) < us);  // wait for the counter to reach the us input in the parameter
}
  // state 1 is duty cycled
void dead_time() {
  while (true) {
    MOSFET_STATE current = get_mosfet_state();
    if (current != last) {
      last = current;
      delay_us(dead_time);
    }
  }
}
