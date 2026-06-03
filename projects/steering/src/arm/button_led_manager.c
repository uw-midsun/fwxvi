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
#define LED_DMA_CHANNEL2_INSTANCE DMA1_Channel2
#define LED_DMA_CH2_IRQn DMA1_Channel2_IRQn
#define LED_DMA_CHANNEL3_INSTANCE DMA1_Channel3
#define LED_DMA_CH3_IRQn DMA1_Channel3_IRQn
#define LED_DMA_CHANNEL4_INSTANCE DMA1_Channel4
#define LED_DMA_CH4_IRQn DMA1_Channel4_IRQn

#ifdef STM32L4P5xx
#define LED_DMAMUX_REQUEST_TIM2_CH3 DMA_REQUEST_TIM2_CH3
#define LED_DMAMUX_REQUEST_TIM2_CH1 DMA_REQUEST_TIM2_CH1
#define LED_DMAMUX_REQUEST_TIM4_CH2 DMA_REQUEST_TIM4_CH2
#else
#define LED_DMAMUX_REQUEST_TIM2_CH3 0U
#define LED_DMAMUX_REQUEST_TIM2_CH1 0U
#define LED_DMAMUX_REQUEST_TIM4_CH2 0U

#endif

/*left board TIM2_ch1*/
/*right board TIM4_ch2*/
/*main board TIM2_ch3*/

static SteeringStorage *steering_storage = NULL;
static ButtonLEDManager s_button_led_manager = { 0U };

static TIMDMABoardManager s_left_board = {.t1_high_ticks = 0U, .t0_high_ticks = 0U, .reset_slots = 0U, .dma_length = 0U, .gpio_address = GPIO_STEERING_LEFT_TURN_LED, .tim_handle = {0U}, .dma_handle = {0U}, .tim_channel = TIM_CHANNEL_2};
static TIMDMABoardManager s_right_board = {.t1_high_ticks = 0U, .t0_high_ticks = 0U, .reset_slots = 0U, .dma_length = 0U, .gpio_address = GPIO_STEERING_RIGHT_TURN_LED, .tim_handle = {0U}, .dma_handle = {0U}, .tim_channel = TIM_CHANNEL_4};
static TIMDMABoardManager s_main_board = {.t1_high_ticks = 0U, .t0_high_ticks = 0U, .reset_slots = 0U, .dma_length = 0U, .gpio_address = GPIO_STEERING_RGB_LIGHTS_PWM_PIN, .tim_handle = {0U}, .dma_handle = {0U}, .tim_channel = TIM_CHANNEL_2};

static uint16_t psc = 0U;
static uint16_t arr = 0U;


void DMA1_Channel2_IRQHandler(void) {
  HAL_DMA_IRQHandler(&s_main_board.dma_handle);
}

void DMA1_Channel3_IRQHandler(void) {
  HAL_DMA_IRQHandler(&s_left_board.dma_handle);
}

void DMA1_Channel4_IRQHandler(void) {
  HAL_DMA_IRQHandler(&s_right_board.dma_handle);
}

static StatusCode button_led_manager_init_dma(DMA_HandleTypeDef *dma_handle, DMA_Channel_TypeDef *channel_instance, uint32_t dmamux_request, uint32_t dma_periph_alignment){
  dma_handle->Instance = channel_instance;
  dma_handle->Init.Request = dmamux_request;
  dma_handle->Init.Direction = DMA_MEMORY_TO_PERIPH;
  dma_handle->Init.PeriphInc = DMA_PINC_DISABLE;
  dma_handle->Init.MemInc = DMA_MINC_ENABLE;
  dma_handle->Init.Mode = DMA_NORMAL;
  dma_handle->Init.Priority = DMA_PRIORITY_HIGH;
  dma_handle->Init.PeriphDataAlignment = dma_periph_alignment;
  dma_handle->Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;

  if (HAL_DMA_Init(dma_handle) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

static StatusCode button_led_manager_init_tim_variables(TIMDMABoardManager *board){

  float counts_per_bit = (float)(board->timer_arr + 1U);
  board->t1_high_ticks = (uint16_t)((SK6812_T1H_US / SK6812_BIT_PERIOD_US) * counts_per_bit + 0.5f);
  board->t0_high_ticks = (uint16_t)((SK6812_T0H_US / SK6812_BIT_PERIOD_US) * counts_per_bit + 0.5f);

  /* Ensure minimum values */
  if (board->t1_high_ticks == 0) board->t1_high_ticks = 1;
  if (board->t1_high_ticks > board->timer_arr) board->t1_high_ticks = board->timer_arr;

  /* T0 can be 0 for very short low pulse */
  if (board->t0_high_ticks > board->timer_arr) board->t0_high_ticks = board->timer_arr / 4;

  /* Reset slots - number of bit-periods to hold line low (~80µs) */
  board->reset_slots = (uint16_t)((SK6812_RESET_US / SK6812_BIT_PERIOD_US) + 0.5f);

  /* Ensure we don't exceed DMA buffer size */
  uint32_t required_size = (NUM_STEERING_BUTTONS * BUTTON_LED_MANAGER_BITS_PER_LED) + board->reset_slots;

  if (required_size > BUTTON_LED_MANAGER_DMA_BUF_LEN) {
    board->reset_slots = (uint16_t)(BUTTON_LED_MANAGER_DMA_BUF_LEN - (NUM_STEERING_BUTTONS * BUTTON_LED_MANAGER_BITS_PER_LED));
  }
  return STATUS_CODE_OK;

}

static uint16_t button_led_manager_period(){
  uint32_t clock_frequency = HAL_RCC_GetPCLK1Freq();
  return (uint16_t)(clock_frequency / (SK6812_BIT_PERIOD_US * (psc + 1)) - 1);
}

static StatusCode button_led_manager_init_tim(TIMDMABoardManager *board){
  TIM_HandleTypeDef tim_handle = board->tim_handle;
  tim_handle.Init.Prescaler = psc;
  tim_handle.Init.Period = arr;
  tim_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
  tim_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  tim_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  tim_handle.Init.RepetitionCounter = 0U;

  if (HAL_TIM_Base_Init(&tim_handle) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  static TIM_OC_InitTypeDef sConfigOC = {0};

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&tim_handle, &sConfigOC, board->tim_channel);

  return STATUS_CODE_OK;
}

static StatusCode button_led_manager_create_dma_buffer(TIMDMABoardManager *board){
 uint32_t idx = 0U;

  /* Build DMA buffer with GRB data for each LED (SK6812 expects GRB) */
  for (uint32_t i = board->steering_buttons_start; i < (board->steering_buttons_finish + 1) && idx < BUTTON_LED_MANAGER_DMA_BUF_LEN - board->reset_slots; ++i) {
    uint8_t g = steering_storage->button_led_manager->led_pixels[i].g;
    uint8_t r = steering_storage->button_led_manager->led_pixels[i].r;
    uint8_t b = steering_storage->button_led_manager->led_pixels[i].b;

    uint32_t grb = ((uint32_t)g << 16U) | ((uint32_t)r << 8U) | (uint32_t)b;

    /* Convert each bit to PWM duty cycle values (MSB first) */
    for (int8_t bit = BUTTON_LED_MANAGER_BITS_PER_LED - 1; bit >= 0 && idx < BUTTON_LED_MANAGER_DMA_BUF_LEN - board->reset_slots; --bit) {
      uint32_t mask = (1UL << bit);
      board->dma_buffer[idx++] = (grb & mask) ? board->t1_high_ticks : board->t0_high_ticks;
    }
  }

  /* Append reset period (low signal for >= 80µs) */
  for (uint32_t s = 0; s < board->reset_slots && idx < BUTTON_LED_MANAGER_DMA_BUF_LEN; ++s) {
    board->dma_buffer[idx++] = 0;
  }

  board->dma_length = (uint16_t)idx;

  return STATUS_CODE_OK;
}

StatusCode button_led_manager_init(SteeringStorage *storage){
  /* Steering storage*/
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  steering_storage = storage;
  steering_storage->button_led_manager = &s_button_led_manager;

  /* Clear all LEDs initially */
  memset(&s_button_led_manager.led_pixels, 0, sizeof(s_button_led_manager.led_pixels));
  s_button_led_manager.needs_update = false;
  s_button_led_manager.is_transmitting = false;

   /* timer init */
  __HAL_RCC_TIM2_CLK_ENABLE();
  __HAL_RCC_TIM4_CLK_ENABLE();  //TODO: Make sure this code acc is going

  arr = button_led_manager_period();

  button_led_manager_init_tim_variables(&s_main_board);
  button_led_manager_init_tim_variables(&s_left_board);
  button_led_manager_init_tim_variables(&s_right_board);

  if(button_led_manager_init_tim(&s_main_board) != STATUS_CODE_OK){
    return STATUS_CODE_INVALID_ARGS;
  }
  if(button_led_manager_init_tim(&s_right_board) != STATUS_CODE_OK){
    return STATUS_CODE_INVALID_ARGS;
  }
  if(button_led_manager_init_tim(&s_left_board) != STATUS_CODE_OK){
    return STATUS_CODE_INVALID_ARGS;
  }

  /*DMA init*/
  __HAL_RCC_DMA1_CLK_ENABLE();
  #ifdef STM32L4P5xx
   __HAL_RCC_DMAMUX1_CLK_ENABLE();
  #endif

  if(button_led_manager_init_dma(&s_main_board.dma_handle, LED_DMA_CHANNEL2_INSTANCE, LED_DMAMUX_REQUEST_TIM2_CH3, DMA_PDATAALIGN_WORD) != STATUS_CODE_OK){
    return STATUS_CODE_INVALID_ARGS;
  }
  if(button_led_manager_init_dma(&s_left_board.dma_handle, LED_DMA_CHANNEL3_INSTANCE, LED_DMAMUX_REQUEST_TIM2_CH1, DMA_PDATAALIGN_WORD) != STATUS_CODE_OK){
    return STATUS_CODE_INVALID_ARGS;
  }
  if(button_led_manager_init_dma(&s_right_board.dma_handle, LED_DMA_CHANNEL4_INSTANCE, LED_DMAMUX_REQUEST_TIM4_CH2, DMA_PDATAALIGN_HALFWORD) != STATUS_CODE_OK){
    return STATUS_CODE_INVALID_ARGS;
  }  

  /* Enable DMA interrupt */
  interrupt_nvic_enable(LED_DMA_CH2_IRQn, INTERRUPT_PRIORITY_HIGH);
  interrupt_nvic_enable(LED_DMA_CH3_IRQn, INTERRUPT_PRIORITY_HIGH);
  interrupt_nvic_enable(LED_DMA_CH4_IRQn, INTERRUPT_PRIORITY_HIGH);

  /* Link TIM and DMA*/
  __HAL_LINKDMA(&s_main_board.tim_handle, hdma[TIM_DMA_ID_CC3], s_main_board.dma_handle);
  __HAL_LINKDMA(&s_left_board.tim_handle, hdma[TIM_DMA_ID_CC3], s_left_board.dma_handle);
  __HAL_LINKDMA(&s_right_board.tim_handle, hdma[TIM_DMA_ID_CC3], s_right_board.dma_handle); //TODO: this part may break

  /*GPIO pin init*/
  if(gpio_init_pin_af(&s_main_board.gpio_address, GPIO_ALTFN_PUSH_PULL, GPIO_ALT1_TIM2) != STATUS_CODE_OK){
    return STATUS_CODE_INVALID_ARGS;
  }
  if(gpio_init_pin_af(&s_left_board.gpio_address, GPIO_ALTFN_PUSH_PULL, GPIO_ALT1_TIM2) != STATUS_CODE_OK){
    return STATUS_CODE_INVALID_ARGS;
  }
  if(gpio_init_pin_af(&s_right_board.gpio_address, GPIO_ALTFN_PUSH_PULL, GPIO_ALT2_TIM4) != STATUS_CODE_OK){
    return STATUS_CODE_INVALID_ARGS;
  }
  
  return STATUS_CODE_OK;
}

StatusCode button_led_manager_update(void){
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if(s_button_led_manager.is_transmitting){
    return STATUS_CODE_INVALID_ARGS;
  }

  button_led_manager_clear_all();

  button_led_manager_create_dma_buffer(&s_main_board);
  button_led_manager_create_dma_buffer(&s_left_board);
  button_led_manager_create_dma_buffer(&s_right_board);

  steering_storage->button_led_manager->is_transmitting = true;

  /* HAL_TIM_PWM_Start_DMA*/
  HAL_TIM_PWM_Start_DMA(&s_main_board.tim_handle, s_main_board.tim_channel, (uint32_t *)s_main_board.dma_buffer, s_main_board.dma_length);
  HAL_TIM_PWM_Start_DMA(&s_left_board.tim_handle, s_left_board.tim_channel, (uint32_t *)s_left_board.dma_buffer, s_left_board.dma_length);
  HAL_TIM_PWM_Start_DMA(&s_right_board.tim_handle, s_right_board.tim_channel, (uint32_t *)s_right_board.dma_buffer, s_right_board.dma_length);

  return STATUS_CODE_OK;
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

bool button_led_manager_is_busy(void) {
  if (steering_storage == NULL) {
    return false;
  }

  return steering_storage->button_led_manager->is_transmitting;
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

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
  /* Add a breakpoint here to verify callback is reached */
  if (htim == &s_main_board.tim_handle) {
    HAL_TIM_PWM_Stop_DMA(&s_main_board.tim_handle, TIM_CHANNEL_3);
    s_button_led_manager.is_transmitting = false;
    s_button_led_manager.needs_update = false;
  }

}