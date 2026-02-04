/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for piezo_am_pwm
 *
 * @date   2025-12-28
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "pwm.h"
#include "status.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_cortex.h"
#include "stm32l4xx_hal_tim.h"
#include "tasks.h"

/* Intra-component Headers */

#define BUZZER_TIMER PWM_TIMER_16
#define BUZZER_CHANNEL PWM_CHANNEL_1
#define BUZZER_GPIO_ALTFN GPIO_ALT14_TIM16
#define BUZZER_DUTY 50U

#define CARRIER_FREQUENCY_KHZ 4  // This is the resonant frequency

#define STEERING_BUZZER_PWM_PIN { .port = GPIO_PORT_A, .pin = 6 }

static GpioAddress s_buzzer_pwm_pin = STEERING_BUZZER_PWM_PIN;

static TIM_HandleTypeDef h_timer = { 0U };

static volatile bool s_is_buzzer_on = false;

static uint16_t prescaler = 79;

static StatusCode toggle_carrier() {  // turns the carrier signal on or off
  if (s_is_buzzer_on) {
    pwm_set_dc(BUZZER_TIMER, BUZZER_DUTY, BUZZER_CHANNEL, false);
    s_is_buzzer_on = false;
  } else {
    pwm_set_dc(BUZZER_TIMER, 0U, BUZZER_CHANNEL, false);
    s_is_buzzer_on = true;
  }
  return STATUS_CODE_OK;
}

static uint16_t get_arr(uint16_t modulation_frequency) {
  uint16_t timer_frequency = 2 * modulation_frequency;
  uint32_t timer_clock = 80000000;  // 80MHz
  uint16_t arr = (uint16_t)(timer_clock / ((prescaler + 1) * timer_frequency)) - 1;

  return arr;
}

StatusCode buzzer_init(void) {
  StatusCode ret = STATUS_CODE_OK;

  ret = gpio_init_pin_af(&s_buzzer_pwm_pin, GPIO_ALTFN_PUSH_PULL, BUZZER_GPIO_ALTFN);
  if (ret != STATUS_CODE_OK) {
    LOG_DEBUG("Error when calling gpio_init_pin_af: %d\r\n", ret);
    delay_ms(10U);
  }
  ret = pwm_init_hz(BUZZER_TIMER, CARRIER_FREQUENCY_KHZ * 1000);
  if (ret != STATUS_CODE_OK) {
    LOG_DEBUG("Error when calling pwm_init_hz: %d\r\n", ret);
    delay_ms(10U);
  }
  ret = pwm_set_dc(BUZZER_TIMER, BUZZER_DUTY, BUZZER_CHANNEL, false);
  if (ret != STATUS_CODE_OK) {
    LOG_DEBUG("Error when calling pwm_set_dc: %d\r\n", ret);
    delay_ms(10U);
  }
  return STATUS_CODE_OK;
}

static void hal_timer_init(uint16_t modulation_frequency) {
  __HAL_RCC_TIM6_CLK_ENABLE();

  uint16_t arr = get_arr(modulation_frequency);

  h_timer.Instance = TIM6;
  h_timer.Init.Prescaler = prescaler;
  h_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
  h_timer.Init.Period = arr;  // This sets the frequency
  h_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  h_timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

  LOG_DEBUG("Attempting to initialize HAL timer\r\n");
  delay_ms(10U);

  if (HAL_TIM_Base_Init(&h_timer) != HAL_OK) {
    LOG_DEBUG("Error initalizing HAL timer!\r\n");
    delay_ms(10U);
  }
  LOG_DEBUG("Finished base init\r\n");
  delay_ms(10U);
  HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *h_timer) {
  if (h_timer->Instance == TIM6) {
    toggle_carrier();
  }
}

TASK(play_notes, TASK_STACK_1024) {
  const uint16_t modulation_frequencies[] = { 131, 147, 165, 175, 196, 220, 247, 262 };
  const uint16_t steady_frequency[] = { 150 };

  const int note_duration = 1000;

  StatusCode ret = buzzer_init();
  if (ret != STATUS_CODE_OK) {
    LOG_DEBUG("Error initializing buzzer\r\n");
    delay_ms(10);
  }

  LOG_DEBUG("Attempting to initialize HAL timer...\r\n");
  delay_ms(10);
  hal_timer_init(modulation_frequencies[0]);
  LOG_DEBUG("HAL timer initialized...\r\n");
  delay_ms(10);

  HAL_TIM_Base_Start_IT(&h_timer);

  LOG_DEBUG("finished start IT\r\n");
  delay_ms(10U);

  while (true) {
    const uint16_t size_test = sizeof(modulation_frequencies) / sizeof(modulation_frequencies[0]);
    for (uint16_t j = 0; j < size_test; ++j) {
      const uint16_t modulation_frequency = modulation_frequencies[j];

      delay_ms(10U);
      uint16_t arr = get_arr(modulation_frequency);

      LOG_DEBUG("Starting to play note of frequency: %d | ARR: %u\r\n", modulation_frequency, arr);
      __HAL_TIM_SET_AUTORELOAD(&h_timer, arr);
      __HAL_TIM_SET_COUNTER(&h_timer, 0);

      delay_ms(note_duration);
    }
    LOG_DEBUG("Finished playing test notes.\r\n");
    delay_ms(10U);
  }
}

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

  tasks_init_task(play_notes, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}

void TIM6_DAC_IRQHandler(void) {
  HAL_TIM_IRQHandler(&h_timer);
}
