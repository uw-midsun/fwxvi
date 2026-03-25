/************************************************************************************************
 * @file    button_led_manager.c
 *
 * @brief   Button LED manager source file - SK6812 Implementation (Fixed for STM32L4P5VET6)
 *
 * @date    2025-11-10
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
/** @brief  Total bit period = 1.2µs (corrected from 1.25µs) */
#define SK6812_BIT_PERIOD_US 1.2f
/** @brief  '1' high time = 0.6µs */
#define SK6812_T1H_US 0.60f
/** @brief  '0' high time = 0.3µs */
#define SK6812_T0H_US 0.30f
/** @brief  Reset low time >= 80µs */
#define SK6812_RESET_US 80.0f

/* DMA configuration for STM32L4P5 - TIM2_CH3 uses DMA1 Channel2 */
/* Note: Channel1 is occupied by ADC, so we use Channel2 */
/* STM32L4P5 uses DMAMUX - TIM2_CH3 is request ID 60 */
#define LED_DMA_CHANNEL_INSTANCE DMA1_Channel2
#define LED_DMA_IRQn DMA1_Channel2_IRQn

#ifdef STM32L4P5xx
#define LED_DMAMUX_REQUEST_TIM2_CH3 DMA_REQUEST_TIM2_CH3
#else
#define LED_DMAMUX_REQUEST_TIM2_CH3 0U
#endif

static uint32_t s_timer_arr = 0U;
static uint16_t s_t1_high_ticks = 0U;
static uint16_t s_t0_high_ticks = 0U;
static uint16_t s_reset_slots = 0U;
static uint16_t s_dma_length = 0U;

static TIM_HandleTypeDef s_tim2_handle = { 0U };
static DMA_HandleTypeDef s_dma_tim2_ch3_handle = { 0U };

static SteeringStorage *steering_storage = NULL;
static ButtonLEDManager s_button_led_manager = { 0U };

static GpioAddress s_button_led_pwm_ctrl = GPIO_STEERING_RGB_LIGHTS_PWM_PIN;

void DMA1_Channel2_IRQHandler(void) {
  HAL_DMA_IRQHandler(&s_dma_tim2_ch3_handle);
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
  /* Add a breakpoint here to verify callback is reached */
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

  /* For SK6812: 1.2µs bit period
   * Strategy: Use PSC=0 for maximum resolution
   * ARR = (tim_clk * 1.2µs) - 1
   *
   * Example: If tim_clk = 80MHz:
   * ARR = (80,000,000 * 1.2e-6) - 1 = 96 - 1 = 95
   */

  uint32_t psc = 0;
  float target_counts = (float)tim_clk * SK6812_BIT_PERIOD_US * 1e-6f;
  uint32_t arr = (uint32_t)(target_counts + 0.5f);  // Round

  if (arr > 0) {
    arr = arr - 1U;
  } else {
    arr = 0;
  }

  /* Clamp to 32-bit (TIM2 is 32-bit timer) */
  if (arr > 0xFFFFFFFFU) {
    arr = 0xFFFFFFFFU;
  }

  /* Program into TIM handle */
  s_tim2_handle.Init.Prescaler = psc;
  s_tim2_handle.Init.Period = arr;

  // /* Generate update event to load new values */
  // s_tim2_handle.Instance->PSC = psc;
  // s_tim2_handle.Instance->ARR = arr;
  // s_tim2_handle.Instance->EGR = TIM_EGR_UG;
  // s_tim2_handle.Instance->CNT = 0;

  s_timer_arr = arr;

  /* Compute high-time ticks for logical '1' and '0' */
  float counts_per_bit = (float)(s_timer_arr + 1U);
  s_t1_high_ticks = (uint16_t)((SK6812_T1H_US / SK6812_BIT_PERIOD_US) * counts_per_bit + 0.5f);
  s_t0_high_ticks = (uint16_t)((SK6812_T0H_US / SK6812_BIT_PERIOD_US) * counts_per_bit + 0.5f);

  /* Ensure minimum values */
  if (s_t1_high_ticks == 0) s_t1_high_ticks = 1;
  if (s_t1_high_ticks > s_timer_arr) s_t1_high_ticks = s_timer_arr;

  /* T0 can be 0 for very short low pulse */
  if (s_t0_high_ticks > s_timer_arr) s_t0_high_ticks = s_timer_arr / 4;

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
#ifdef STM32L4P5xx
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
#endif

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
  s_dma_tim2_ch3_handle.Init.Request = LED_DMAMUX_REQUEST_TIM2_CH3;
  s_dma_tim2_ch3_handle.Init.Direction = DMA_MEMORY_TO_PERIPH;
  s_dma_tim2_ch3_handle.Init.PeriphInc = DMA_PINC_DISABLE;
  s_dma_tim2_ch3_handle.Init.MemInc = DMA_MINC_ENABLE;
  s_dma_tim2_ch3_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  s_dma_tim2_ch3_handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD; /**< Buffer is uint16_t */
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

  /* Initialize GPIO pin for PWM output - TIM2_CH3 can be PA2 or PB10 */
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

  /* CRITICAL: Cast to uint32_t* for CCR register (16-bit values but 32-bit register access) */
  HAL_StatusTypeDef hal_status = HAL_TIM_PWM_Start_DMA(&s_tim2_handle, TIM_CHANNEL_3, (uint32_t *)steering_storage->button_led_manager->dma_buffer, s_dma_length);

  if (hal_status != HAL_OK) {
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
