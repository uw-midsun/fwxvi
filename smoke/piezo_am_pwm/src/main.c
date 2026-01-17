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
#include "software_timer.h"
#include "status.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_tim.h"
#include "tasks.h"

/* Intra-component Headers */

#define BUZZER_TIMER PWM_TIMER_4
#define BUZZER_CHANNEL PWM_CHANNEL_1
#define BUZZER_GPIO_ALTFN GPIO_ALT2_TIM4
#define BUZZER_DUTY 50U
#define BUZZER_BEEP_DURATION_MS 250U

#define CARRIER_FREQUENCY 4  // in kHz

#define STEERING_BUZZER_PWM_PIN { .port = GPIO_PORT_D, .pin = 12 }

static GpioAddress s_buzzer_pwm_pin = STEERING_BUZZER_PWM_PIN;

static TIM_HandleTypeDef h_timer = { 0U };

StatusCode toggle_carrier() {  // turns the carrier signal on or off
  const uint16_t dutyCycle = pwm_get_dc(BUZZER_TIMER, BUZZER_CHANNEL);
  if (dutyCycle == BUZZER_DUTY) {
    pwm_set_dc(BUZZER_TIMER, 0U, BUZZER_CHANNEL, false);
  } else if (dutyCycle == 0U) {
    pwm_set_dc(BUZZER_TIMER, BUZZER_DUTY, BUZZER_CHANNEL, false);
  } else {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  return STATUS_CODE_OK;
}

int get_arr(int modulation_frequency) {
  int timer_frequency = 2 * modulation_frequency;
  int timer_clock = 48000000;  // 48MHz
  int prescaler = 47;
  int arr = (timer_clock / (prescaler + 1) / timer_frequency) - 1;

  return arr;
}

void Timer_Frequency_Init(int modulation_frequency) {
  gpio_init_pin_af(&s_buzzer_pwm_pin, GPIO_ALTFN_PUSH_PULL, BUZZER_GPIO_ALTFN);

  int prescaler = 79;
  int arr = get_arr(modulation_frequency);

  __HAL_RCC_TIM2_CLK_ENABLE();

  h_timer.Instance = TIM_CHANNEL_1;
  h_timer.Instance->CR1 |= TIM_CR1_ARPE;
  h_timer.Init.Prescaler = prescaler;
  h_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
  h_timer.Init.Period = arr;  // This sets the frequency
  h_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  h_timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

  if (HAL_TIM_Base_Init(&h_timer) != HAL_OK) {
    LOG_DEBUG("Error initalizing HAL timer!");
  }
  HAL_TIM_Base_Start_IT(&h_timer);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *h_timer) {
  toggle_carrier();
}

TASK(play_notes, TASK_STACK_1024) {
  const int modulation_frequencies[] = { 100, 350, 1391, 2000 };
  const int note_duration = 100;

  const uint16_t carrier_period = 1000 / CARRIER_FREQUENCY;
  pwm_init_hz(BUZZER_TIMER, carrier_period * 1000);
  Timer_Frequency_Init(modulation_frequencies[0]);

  while (true) {
    const int size_test = sizeof(modulation_frequencies) / sizeof(modulation_frequencies[0]);
    for (int j = 0; j < size_test; ++j) {
      const int modulation_frequency = modulation_frequencies[j];

      LOG_DEBUG("Starting to play note of frequency: %d\n", modulation_frequency);
      int arr = get_arr(modulation_frequency);
      __HAL_TIM_SET_AUTORELOAD(&h_timer, arr);
      delay_ms(note_duration);
    }
    LOG_DEBUG("Finished playing test notes.\n");
  }
}

// TASK(debug_statements, TASK_STACK_1024) {
//   while (true) {
//     const int delay = 1;
//     delay_ms(delay);
//     uint16_t dutyCycle = pwm_get_dc(BUZZER_TIMER, BUZZER_CHANNEL);
//     LOG_DEBUG("Duty cycle (sampled every %d milliseconds): %d\n", delay, dutyCycle);
//   }
// }

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

  // tasks_init_task(debug_statements, TASK_PRIORITY(3), NULL);
  tasks_init_task(play_notes, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
