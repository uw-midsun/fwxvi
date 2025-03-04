/************************************************************************************************
 * @file   pwm.c
 *
 * @brief  PWM Library Source code
 *
 * @date   2025-03-01
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "stm32l433xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_tim.h"

/* Intra-component Headers */
#include "gpio.h"
#include "log.h"
#include "pwm.h"
#include "status.h"

#define NUM_CHANNELS 4

static TIM_HandleTypeDef s_timer_handle[NUM_PWM_TIMERS];
static uint16_t s_period_us[NUM_PWM_TIMERS] = { 0 };

static void s_enable_periph_clock(PwmTimer timer) {
  switch (timer) {
    case PWM_TIMER_1:
      __HAL_RCC_TIM1_CLK_ENABLE();
      break;
    case PWM_TIMER_2:
      __HAL_RCC_TIM2_CLK_ENABLE();
      break;
    case PWM_TIMER_15:
      __HAL_RCC_TIM15_CLK_ENABLE();
      break;
    case PWM_TIMER_16:
      __HAL_RCC_TIM16_CLK_ENABLE();
      break;
    default:
      break;
  }
}

StatusCode pwm_init(PwmTimer timer, uint16_t period_us) {
  if (timer >= NUM_PWM_TIMERS || period_us == 0) {
    LOG_DEBUG("invalid timer");
    return STATUS_CODE_INVALID_ARGS;
  }

  s_enable_periph_clock(timer);

  switch (timer) {
    case PWM_TIMER_1:
      s_timer_handle[timer].Instance = TIM1;
      break;
    case PWM_TIMER_2:
      s_timer_handle[timer].Instance = TIM2;
      break;
    case PWM_TIMER_15:
      s_timer_handle[timer].Instance = TIM15;
      break;
    case PWM_TIMER_16:
      s_timer_handle[timer].Instance = TIM16;
      break;
    default:
      return STATUS_CODE_INVALID_ARGS;
  }

  uint32_t clock_freq;
  /* Based on Pg 36 of datasheet. Clock tree */
  if (timer == PWM_TIMER_2) {
    clock_freq = HAL_RCC_GetPCLK1Freq();
  } else {
    clock_freq = HAL_RCC_GetPCLK2Freq();
  }

  s_timer_handle[timer].Init.Prescaler = (clock_freq / 1000000U) - 1U;
  s_timer_handle[timer].Init.CounterMode = TIM_COUNTERMODE_UP;
  s_timer_handle[timer].Init.Period = period_us - 1U;
  s_timer_handle[timer].Init.ClockDivision = 0U;
  s_timer_handle[timer].Init.RepetitionCounter = 0U;

  if (HAL_TIM_PWM_Init(&s_timer_handle[timer]) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  s_period_us[timer] = period_us;
  return STATUS_CODE_OK;
}

StatusCode pwm_set_pulse(PwmTimer timer, uint16_t pulse_width_us, PwmChannel channel, bool n_channel_en) {
  if (timer >= NUM_PWM_TIMERS || pulse_width_us > s_period_us[timer] || s_period_us[timer] == 0) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Refer to Table 10 Pg 44 of STM32L433CCU6 Datasheet */

  bool channel_supported = false;
  switch (timer) {
    case PWM_TIMER_1:
      /* TIM1 supports all 4 channels */
      channel_supported = (channel < 4);
      break;
    case PWM_TIMER_2:
      /* TIM2 supports all 4 channels */
      channel_supported = (channel < 4);
      break;
    case PWM_TIMER_15:
      /* TIM15 supports channels 1 and 2 */
      channel_supported = (channel < 2);
      break;
    case PWM_TIMER_16:
      /* TIM16 supports only channel 1 */
      channel_supported = (channel == 0);
      break;
    default:
      channel_supported = false;
  }

  if (!channel_supported) {
    LOG_DEBUG("Channel %d not supported on Timer %d", channel, timer);
    return STATUS_CODE_INVALID_ARGS;
  }

  bool has_complementary = true;
  if (timer == PWM_TIMER_2) {
    has_complementary = false;
  }

  TIM_OC_InitTypeDef output_compare_config = { 0 };
  output_compare_config.Pulse = pulse_width_us;
  output_compare_config.OCMode = TIM_OCMODE_PWM1;
  output_compare_config.OCPolarity = TIM_OCPOLARITY_HIGH;
  output_compare_config.OCFastMode = TIM_OCFAST_DISABLE;
  output_compare_config.OCIdleState = TIM_OCIDLESTATE_RESET;

  if (has_complementary) {
    output_compare_config.OCNPolarity = n_channel_en ? TIM_OCNPOLARITY_HIGH : TIM_OCNPOLARITY_LOW;
    output_compare_config.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  }

  if (HAL_TIM_PWM_ConfigChannel(&s_timer_handle[timer], &output_compare_config, channel * 4U) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  HAL_TIM_PWM_Start(&s_timer_handle[timer], channel * 4U);

  /* Enables N chanenl if requested and is available */
  if (has_complementary && n_channel_en) {
    HAL_TIMEx_PWMN_Start(&s_timer_handle[timer], channel * 4U);
  }

  return STATUS_CODE_OK;
}

StatusCode pwm_set_dc(PwmTimer timer, uint16_t dc, PwmChannel channel, bool n_channel_en) {
  if (timer >= NUM_PWM_TIMERS || dc > 100) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint16_t pulse_width;
  if (dc == 0U) {
    /* Prevent divide by 0 */
    pulse_width = 0U;
  } else {
    pulse_width = ((s_period_us[timer]) * dc) / 100;
    /* Avoid overflow at 100% duty cycle */
    if (pulse_width >= s_period_us[timer]) {
      pulse_width = s_period_us[timer] - 1;
    }
  }

  return pwm_set_pulse(timer, pulse_width, channel, n_channel_en);
}

uint16_t pwm_get_dc(PwmTimer timer, PwmChannel channel) {
  uint32_t pulse = 0U;

  /* CCRX stores the value that is loaded into the capture/compare 2 register */

  switch (channel * 4U) {
    case TIM_CHANNEL_1:
      pulse = s_timer_handle[timer].Instance->CCR1;
      break;
    case TIM_CHANNEL_2:
      pulse = s_timer_handle[timer].Instance->CCR2;
      break;
    case TIM_CHANNEL_3:
      pulse = s_timer_handle[timer].Instance->CCR3;
      break;
    case TIM_CHANNEL_4:
      pulse = s_timer_handle[timer].Instance->CCR4;
      break;
  }

  if (s_period_us[timer] == 0) {
    return 0;
  }

  return (pulse * 100) / s_period_us[timer];
}

uint16_t pwm_get_period(PwmTimer timer) {
  return (timer < NUM_PWM_TIMERS) ? s_period_us[timer] : 0;
}
