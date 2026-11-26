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
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"

static bool all_gpio_open(void);  // FOR TEST CASES

/* Intra-component Headers */
#include "horn_driver.h"

#define TIM1_CH1 8      // PA8 Q1
#define TIM1_CH1N 7     // PA7 Q3
#define TIM1_CH2 9      // PA9 Q2
#define TIM1_CH2N 0     // PB0 Q4
#define DEAD_TIME_US 5     // in us
#define DUTY_CYCLE_PER 0.5  // 50% duty cycle

void dead_time_run();
void delay_us(uint16_t us);
MosfetState get_mosfet_state();

MosfetState last = ALL_OFF;

GpioAddress Q1 = {
  .pin = TIM1_CH1,
  .port = GPIO_PORT_A,
};
GpioAddress Q3 = {
  .pin = TIM1_CH1N,
  .port = GPIO_PORT_A,
};
GpioAddress Q2 = {
  .pin = TIM1_CH2,
  .port = GPIO_PORT_A,
};
GpioAddress Q4 = {
  .pin = TIM1_CH2N,
  .port = GPIO_PORT_B,
};

TIM_BreakDeadTimeConfigTypeDef dead_time_config = {
  TIM_OSSR_DISABLE,
  TIM_OSSI_DISABLE,
  TIM_LOCKLEVEL_OFF,
  0xD2,
  TIM_BREAK_DISABLE,
  TIM_BREAKPOLARITY_LOW,
  0x0,
  TIM_BREAK2_DISABLE,
  TIM_BREAK2POLARITY_LOW,
  0x0,
  TIM_AUTOMATICOUTPUT_DISABLE
};

// this should be here for a start up because when we do it in the other fcn it might/shouldn't read not? research more
TASK(horn_driver, TASK_STACK_2048) {
  if (all_gpio_open()) {
    LOG_DEBUG("pins are open at startup!");
  }  // NOTE TO JOHN OR NEXT USER PLEASE DOUBLE CHECK HOW TO FIND THIS MESSAGE NEXT DAY OF TESTING
    // MINICOM INFO:git  https://uwmidsun.atlassian.net/wiki/spaces/Firmware/pages/3263103250/Minicom+Instructions
  
  
  HAL_TIMEx_ConfigBreakDeadTime(&h_timer,&dead_time_config);
  while (true) {
    gpio_init_pin_af(&Q1, GPIO_INPUT_PULL_DOWN, GPIO_ALT1_TIM1);
    gpio_init_pin_af(&Q3, GPIO_INPUT_PULL_DOWN, GPIO_ALT1_TIM1);
    gpio_init_pin_af(&Q2, GPIO_INPUT_PULL_DOWN, GPIO_ALT1_TIM1);
    gpio_init_pin_af(&Q4, GPIO_INPUT_PULL_DOWN, GPIO_ALT1_TIM1);
  };
}

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
  horn_driver_init();

  tasks_init_task(horn_driver, TASK_PRIORITY(3), NULL);
  
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}

MosfetState get_mosfet_state() {
  GpioState Q1_state = gpio_get_state(&Q1);  // Q1
  GpioState Q3_state = gpio_get_state(&Q3);  // Q3
  GpioState Q2_state = gpio_get_state(&Q2);  // Q2
  GpioState Q4_state = gpio_get_state(&Q4);  // Q4

  if (Q1_state == GPIO_STATE_HIGH && Q4_state == GPIO_STATE_HIGH) {
    return Q1_Q4_ON;
  } else if (Q2_state == GPIO_STATE_HIGH && Q3_state == GPIO_STATE_HIGH) {
    return Q2_Q3_ON;
  }
  return ALL_OFF;
}

void delay_us(uint16_t us) {
  __HAL_TIM_SET_COUNTER(&h_timer, 0);  // set the counter value a 0
  while (__HAL_TIM_GET_COUNTER(&h_timer) < us);  // wait for the counter to reach the us input in the parameter
}

// state 1 is duty cycled
void dead_time_run() {
  while (true) {
    MosfetState current = get_mosfet_state();
    if (current != last) {
      last = current;
      delay_us(DEAD_TIME_US);
    }
  }
}

// CHECKS IF ALL PINS ARE STARTING OFF
static bool all_gpio_open(void) {
  return gpio_get_state(&Q1) == GPIO_STATE_LOW &&  // Q1
         gpio_get_state(&Q3) == GPIO_STATE_LOW &&  // Q3
         gpio_get_state(&Q2) == GPIO_STATE_LOW &&  // Q2
         gpio_get_state(&Q4) == GPIO_STATE_LOW;    // Q4
}