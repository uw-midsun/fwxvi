/************************************************************************************************
 * @file    button_led_manager.c
 *
 * @brief   Button LED manager source file - SK6812 Implementation
 *
 * @date    2025-09-27
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "gpio.h"
#include "interrupts.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_dma.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_tim.h"

/* Intra-component Headers */
#include "button_led_manager.h"
#include "steering.h"
#include "steering_hw_defs.h"

/* SK6812 timing parameters (from datasheet) */
/** @brief  Total bit period = 1.25µs */
#define SK6812_BIT_PERIOD_US 1.25f
/** @brief  '1' high time = 0.6µs */
#define SK6812_T1H_US 0.60f
/** @brief  '0' high time = 0.3µs */
#define SK6812_T0H_US 0.30f
/** @brief  Reset low time >= 80µs */
#define SK6812_RESET_US 80.0f

#define LED_DMA_CHANNEL_INSTANCE DMA1_Channel2
#define LED_DMA_IRQn DMA1_Channel2_IRQn
#define LED_DMA_REQUEST DMA_REQUEST_8

static uint32_t s_timer_arr = 0U;
static uint16_t s_t1_high_ticks = 0U;
static uint16_t s_t0_high_ticks = 0U;
static uint16_t s_reset_slots = 0U;
static uint16_t s_dma_length = 0U;

static TIM_HandleTypeDef s_tim2_handle = { 0U };
static DMA_HandleTypeDef s_dma_tim2_ch3_handle = { 0U };

static SteeringStorage *steering_storage = NULL;
static ButtonLEDManager s_button_led_manager = { 0U };

static GpioAddress s_button_led_pwm_ctrl = STEERING_RGB_LIGHTS_PWM_PIN;

void DMA1_Channel2_IRQHandler(void) {
  HAL_DMA_IRQHandler(&s_dma_tim2_ch3_handle);
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
  if (htim == &s_tim2_handle) {
    HAL_TIM_PWM_Stop_DMA(&s_tim2_handle, TIM_CHANNEL_3);
    s_button_led_manager.is_transmitting = false;
    s_button_led_manager.needs_update = false;
  }
}

static void button_led_manager_compute_timing_from_clock(void) {
  /* Get PCLK1 and APB1 prescaler state */
  RCC_ClkInitTypeDef clkcfg;
  uint32_t latency;
  HAL_RCC_GetClockConfig(&clkcfg, &latency);

  uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
  uint64_t tim_clk = pclk1;

  /* Timer clocks run at PCLK * 2 if APB prescaler != 1 (per RM) */
  if (clkcfg.APB1CLKDivider != RCC_HCLK_DIV1) {
    tim_clk = (uint64_t)pclk1 * 2U;
  }

  /* Compute target tick counts for one SK6812 bit period */
  double target_counts_d = (double)tim_clk * (SK6812_BIT_PERIOD_US * 1e-6);

  /* Find PSC and ARR such that (ARR+1) * (PSC+1) ~= target_counts_d */
  uint32_t psc = 0;
  uint32_t arr = 0;

  if (target_counts_d <= 1.0) {
    psc = 0;
    arr = 1;
  } else if (target_counts_d <= 65535.0) {
    psc = 0;
    arr = (uint32_t)(target_counts_d + 0.5); /* round */
  } else {
    /* Compute ceil(target_counts / 65536) - 1 for prescaler */
    uint32_t needed = (uint32_t)((target_counts_d + 65535.0) / 65536.0);
    if (needed == 0) needed = 1U;
    psc = (needed > 0) ? (needed - 1U) : 0U;
    double arr_d = ((double)tim_clk / (double)(psc + 1U)) * (SK6812_BIT_PERIOD_US * 1e-6);
    if (arr_d < 1.0)
      arr = 1U;
    else if (arr_d > 65535.0)
      arr = 65535U;
    else
      arr = (uint32_t)(arr_d + 0.5);
  }

  /* Ensure ARR fits in 16-bit */
  if (arr == 0) arr = 1;
  if (arr > 0xFFFFU) arr = 0xFFFFU;

  uint32_t arr_for_tim = (arr > 0) ? (arr - 1U) : 0U;

  /* Program into TIM handle */
  s_tim2_handle.Init.Prescaler = psc;
  s_tim2_handle.Init.Period = arr_for_tim;

  /* Set hardware registers directly */
  __HAL_TIM_SET_PRESCALER(&s_tim2_handle, s_tim2_handle.Init.Prescaler);
  __HAL_TIM_SET_AUTORELOAD(&s_tim2_handle, s_tim2_handle.Init.Period);
  __HAL_TIM_SET_COUNTER(&s_tim2_handle, 0);

  s_timer_arr = (uint32_t)s_tim2_handle.Init.Period;

  /* Compute high-time ticks for logical '1' and '0' */
  double counts_per_bit = (double)(s_timer_arr + 1U);
  s_t1_high_ticks = (uint16_t)((SK6812_T1H_US / SK6812_BIT_PERIOD_US) * counts_per_bit + 0.5);
  s_t0_high_ticks = (uint16_t)((SK6812_T0H_US / SK6812_BIT_PERIOD_US) * counts_per_bit + 0.5);

  if (s_t1_high_ticks == 0) s_t1_high_ticks = 1;
  if (s_t0_high_ticks == 0) s_t0_high_ticks = 0;

  /* Reset slots - number of bit-periods to hold line low (~80µs) */
  s_reset_slots = (uint16_t)((SK6812_RESET_US / SK6812_BIT_PERIOD_US) + 0.5f);

  /* Ensure we don't exceed DMA buffer size */
  uint32_t required_size = (NUM_STEERING_BUTTONS * BUTTON_LED_MANAGER_BITS_PER_LED) + s_reset_slots;

  if (required_size > BUTTON_LED_MANAGER_DMA_BUF_LEN) {
    s_reset_slots = (uint16_t)(BUTTON_LED_MANAGER_DMA_BUF_LEN - (NUM_STEERING_BUTTONS * BUTTON_LED_MANAGER_BITS_PER_LED));
  }
}

static StatusCode button_led_manager_init_timer_dma(void) {
  /* Enable clocks */
  __HAL_RCC_TIM2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* Compute timing parameters */
  button_led_manager_compute_timing_from_clock();

  /* TIM base init */
  s_tim2_handle.Instance = TIM2;
  s_tim2_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
  s_tim2_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  s_tim2_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  if (HAL_TIM_Base_Init(&s_tim2_handle) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  if (HAL_TIM_PWM_Init(&s_tim2_handle) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  /* PWM channel configuration */
  TIM_OC_InitTypeDef sConfigOC = { 0U };
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&s_tim2_handle, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  /* DMA initialization */
  s_dma_tim2_ch3_handle.Instance = LED_DMA_CHANNEL_INSTANCE;
#if defined(DMA_REQUEST_0) || defined(DMA_REQUEST_1) || defined(DMA_REQUEST_8)
  s_dma_tim2_ch3_handle.Init.Request = LED_DMA_REQUEST;
#endif
  s_dma_tim2_ch3_handle.Init.Direction = DMA_MEMORY_TO_PERIPH;
  s_dma_tim2_ch3_handle.Init.PeriphInc = DMA_PINC_DISABLE;
  s_dma_tim2_ch3_handle.Init.MemInc = DMA_MINC_ENABLE;
  s_dma_tim2_ch3_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  s_dma_tim2_ch3_handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  s_dma_tim2_ch3_handle.Init.Mode = DMA_NORMAL;
  s_dma_tim2_ch3_handle.Init.Priority = DMA_PRIORITY_HIGH;

  if (HAL_DMA_Init(&s_dma_tim2_ch3_handle) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  /* Link DMA to TIM handle */
  __HAL_LINKDMA(&s_tim2_handle, hdma[TIM_DMA_ID_CC3], s_dma_tim2_ch3_handle);

  /* Enable DMA interrupt */
  interrupt_nvic_enable(LED_DMA_IRQn, INTERRUPT_PRIORITY_HIGH);

  return STATUS_CODE_OK;
}

static void button_led_manager_build_dma_buffer(void) {
  uint32_t idx = 0U;

  /* Build DMA buffer with GRB data for each LED (SK6812 expects GRB) */
  for (uint32_t i = 0; i < NUM_STEERING_BUTTONS && idx < BUTTON_LED_MANAGER_DMA_BUF_LEN - s_reset_slots; ++i) {
    uint8_t g = steering_storage->button_led_manager->led_pixels[i].g;
    uint8_t r = steering_storage->button_led_manager->led_pixels[i].r;
    uint8_t b = steering_storage->button_led_manager->led_pixels[i].b;

    uint32_t grb = ((uint32_t)g << 16U) | ((uint32_t)r << 8U) | (uint32_t)b;

    /* Convert each bit to PWM duty cycle values (MSB first) */
    for (int8_t bit = BUTTON_LED_MANAGER_BITS_PER_LED - 1; bit >= 0 && idx < BUTTON_LED_MANAGER_DMA_BUF_LEN - s_reset_slots; --bit) {
      uint32_t mask = (1UL << bit);
      steering_storage->button_led_manager->dma_buffer[idx++] = (grb & mask) ? s_t1_high_ticks : s_t0_high_ticks;
    }
  }

  /* Append reset period (low signal for >= 80µs) */
  for (uint32_t s = 0; s < s_reset_slots && idx < BUTTON_LED_MANAGER_DMA_BUF_LEN; ++s) {
    steering_storage->button_led_manager->dma_buffer[idx++] = 0;
  }

  s_dma_length = (uint16_t)idx;
}

StatusCode button_led_manager_init(SteeringStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  steering_storage = storage;
  steering_storage->button_led_manager = &s_button_led_manager;

  /* Clear all LEDs initially */
  memset(&s_button_led_manager.led_pixels, 0, sizeof(s_button_led_manager.led_pixels));
  s_button_led_manager.needs_update = false;
  s_button_led_manager.is_transmitting = false;

  /* Initialize GPIO pin for PWM output */
  StatusCode status = gpio_init_pin_af(&s_button_led_pwm_ctrl, GPIO_ALTFN_PUSH_PULL, GPIO_ALT1_TIM2);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  return button_led_manager_init_timer_dma();
}

StatusCode button_led_manager_set_color(SteeringButtons button, LEDPixels color_code) {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (button >= NUM_STEERING_BUTTONS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  memcpy(&steering_storage->button_led_manager->led_pixels[button], &color_code, sizeof(LEDPixels));
  steering_storage->button_led_manager->needs_update = true;

  return STATUS_CODE_OK;
}

StatusCode button_led_manager_update(void) {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (steering_storage->button_led_manager->is_transmitting) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  /* Build DMA buffer with current LED colors */
  button_led_manager_build_dma_buffer();

  /* Start DMA transmission */
  steering_storage->button_led_manager->is_transmitting = true;

  if (HAL_TIM_PWM_Start_DMA(&s_tim2_handle, TIM_CHANNEL_3, (uint32_t *)steering_storage->button_led_manager->dma_buffer, s_dma_length) != HAL_OK) {
    steering_storage->button_led_manager->is_transmitting = false;
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

bool button_led_manager_is_busy(void) {
  if (steering_storage == NULL) {
    return false;
  }

  return steering_storage->button_led_manager->is_transmitting;
}

StatusCode button_led_manager_clear_all(void) {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  /* Set all LEDs to black (off) */
  memset(steering_storage->button_led_manager->led_pixels, 0U, sizeof(steering_storage->button_led_manager->led_pixels));
  steering_storage->button_led_manager->needs_update = true;

  return STATUS_CODE_OK;
}
