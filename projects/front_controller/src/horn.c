/************************************************************************************************
 * @file    horn.c
 *
 * @brief   Source file for horn manager
 *
 * @date    2025-11-17
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <math.h>

/* Inter-component Headers */
#include "adc.h"
#include "front_controller_getters.h"
#include "gpio.h"
#include "log.h"

/* Intra-component Headers */
#include "front_controller_hw_defs.h"
#include "horn.h"

static GpioAddress s_horn_gpio = GPIO_FRONT_CONTROLLER_HORN_LS_ENABLE;

StatusCode horn_init() {
  gpio_init_pin(&s_horn_gpio, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  pwm_set_dc(PWM_TIMER_1, DUTY_CYCLE_PER, PWM_CHANNEL_1, true);  // Q1 on first, Q3 off first
  LL_TIM_OC_SetPolarity(TIM1, PWM_CHANNEL_2, LL_TIM_OCPOLARITY_HIGH);
  pwm_set_dc(PWM_TIMER_1,DUTY_CYCLE_PER, PWM_CHANNEL_2, true);  // Q2 on first, Q4 off first
  // need to invert second pwm since Q4 should be on first
  HAL_TIM_Base_Start_IT(&h_timer);
  return STATUS_CODE_OK;
}
