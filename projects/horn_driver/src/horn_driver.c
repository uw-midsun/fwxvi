/************************************************************************************************
 * @file   horn_driver.c
 *
 * @brief  Header file for horn_driver
 *
 * @date   2026-11-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "status.h"
#include "log.h"

/* Intra-component Headers */
#include "horn_driver.h"
#define PERIOD_MS 250      // 4kHz
// macros
static TIM_HandleTypeDef h_timer = {0U};

static TIM_OC_InitTypeDef s_config = {
  //TIM_OCMODE_TIMING is default
  //TIM_OCMODE_PWM2 is the polarity inversion if we want to use it
  .OCMode = TIM_OCMODE_PWM1,
  //currently for 50% duty cycle the ARR is 19999.
  .Pulse = 10000U,
  .OCPolarity = TIM_OCPOLARITY_HIGH,
  .OCNPolarity = TIM_OCNPOLARITY_HIGH,
  .OCFastMode = TIM_OCFAST_DISABLE,
  .OCIdleState = TIM_OCIDLESTATE_RESET,
  .OCNIdleState = TIM_OCNIDLESTATE_RESET,
};

// calculated using 5 microseconds of deadtime
static TIM_BreakDeadTimeConfigTypeDef bdtr_config = {
  .OffStateRunMode = TIM_OSSR_DISABLE,
  .OffStateIDLEMode = TIM_OSSI_DISABLE,
  .LockLevel = TIM_LOCKLEVEL_OFF,
  .DeadTime = 0xD2,
  .BreakState = TIM_BREAK_DISABLE,
  .BreakPolarity = TIM_BREAKPOLARITY_HIGH,
  .BreakFilter = 0,
  .Break2State = TIM_BREAK2_DISABLE,
  .Break2Polarity = TIM_BREAK2POLARITY_HIGH,
  .Break2Filter = 0,
  .AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE,
};


StatusCode horn_driver_init() {
  h_timer.Instance = TIM1;
  h_timer.Init.Prescaler = 0U; // some number need to figure out;
  h_timer.Init.Period    = 19999U;
  

  HAL_TIM_OC_ConfigChannel(&h_timer, &s_config, TIM1_CH2);

  HAL_TIMEx_ConfigBreakDeadTime(&h_timer,&bdtr_config);

  if (HAL_TIM_Base_Init(&h_timer) != HAL_OK) {
    LOG_DEBUG("Error in HAL init\r\n");
    return STATUS_CODE_INTERNAL_ERROR;
  }
  
  if (HAL_TIM_PWM_Init(&h_timer) != HAL_OK) {
    LOG_DEBUG("Error in PWM init");
    return STATUS_CODE_INTERNAL_ERROR;
  }
  //__HAL_TIM_MOE_ENABLE(h_timer);

  HAL_TIM_PWM_Start(&h_timer, TIM1_CH1);
  HAL_TIMEx_PWMN_Start(&h_timer,TIM1_CH1); // do we need to do TIM1_CH1N?
  HAL_TIM_PWM_Start(&h_timer, TIM1_CH2);
  HAL_TIMEx_PWMN_Start(&h_timer, TIM1_CH2); // same question as above
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode horn_driver_enable(bool is_enabled) {
  return STATUS_CODE_UNIMPLEMENTED;
}



/** @} */