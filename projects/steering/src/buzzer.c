/************************************************************************************************
 * @file    buzzer.c
 *
 * @brief   Source file for the steering buzzer with melody support
 *
 * @date    2025-07-28
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>

/* Inter-component Headers */
#include "delay.h"
#include "global_enums.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "pwm.h"
#include "software_timer.h"

#include "status.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_cortex.h"
#include "stm32l4xx_hal_tim.h"
#include "tasks.h"

/* Intra-component Headers */
#include "buzzer.h"
#include "steering_hw_defs.h"

#define BUZZER_TIMER PWM_TIMER_16
#define BUZZER_CHANNEL PWM_CHANNEL_1
#define BUZZER_GPIO_ALTFN GPIO_ALT14_TIM16
#define BUZZER_DUTY 50U
#define BUZZER_BEEP_DURATION_MS 250U

#define CARRIER_FREQUENCY_KHZ 4  // This is the resonant frequency

/* Amount of PWM divisions in one clock cycle - we will be able to support PWM in increments of (100 / PWM_DIV) */
#define PWM_DIV 5
/* Amount of PWM divs in one high phase, so the duty cycle will be equal to (100 / PWM_DIV) * PWM_HIGH_DIV_AMT */
#define PWM_HIGH_DIV_AMT 4
#define PWM_LOW_DIV_AMT PWM_DIV - PWM_HIGH_DIV_AMT

#define STEERING_BUZZER_PWM_PIN \
  { .port = GPIO_PORT_A, .pin = 6 }

static GpioAddress s_buzzer_pwm_pin = STEERING_BUZZER_PWM_PIN;

static TIM_HandleTypeDef h_timer = { 0U };

static volatile bool s_is_buzzer_on = false;
static volatile int pwm_counter = 0;
static bool amplitude_modulation_enabled = false;
static bool isNoteRest = false;

static uint16_t prescaler = 79;

#define BUZZER_DEBUG 0U

/* Upbeat ascending startup jingle */
static Note MELODY_STARTUP[] = { { NOTE_C5, 100 }, { NOTE_E5, 100 }, { NOTE_G5, 100 }, { NOTE_C6, 150 }, { NOTE_REST, 50 }, { NOTE_G5, 120 }, { NOTE_C6, 150 }, { NOTE_E6, 250 }, { NOTE_REST, 0 } };

/* Descending "uh-oh" error sound */
static Note MELODY_ERROR[] = { { NOTE_A5, 150 }, { NOTE_F5, 150 }, { NOTE_D5, 150 }, { NOTE_A4, 300 }, { NOTE_REST, 0 } };

/* Triumphant success fanfare */
static Note MELODY_SUCCESS[] = { { NOTE_C5, 100 }, { NOTE_E5, 100 }, { NOTE_G5, 100 }, { NOTE_C6, 150 }, { NOTE_REST, 50 }, { NOTE_G5, 100 }, { NOTE_C6, 250 }, { NOTE_REST, 0 } };

/* Turn signal click high*/
static Note TURN_SIGNAL_CLICK_HIGH[] = { { NOTE_C8, 50 }, { NOTE_REST, 0 } };

/* Turn signal click low*/
static Note TURN_SIGNAL_CLICK_LOW[] = { { NOTE_C7, 50 }, { NOTE_REST, 0 } };

static Note MELODY_DRIVE_STATE[] = { { NOTE_C6, 100 }, { NOTE_REST, 0 } };

static Note MELODY_NEUTRAL_STATE[] = { { NOTE_E6, 100 }, { NOTE_REST, 0 } };

static Note MELODY_REVERSE_STATE[] = { { NOTE_G6, 100 }, { NOTE_REST, 0 } };

static Note MELODY_INVALID_STATE[] = { { NOTE_C5, 100 }, { NOTE_REST, 50 }, { NOTE_C5, 100 }, { NOTE_REST, 0 } };

static Note MELODY_REGEN_ON[] = { { NOTE_C4, 30 }, { NOTE_D4, 30 }, { NOTE_E4, 30 }, { NOTE_F4, 30 }, { NOTE_G4, 30 }, { NOTE_A4, 30 },
                                  { NOTE_B4, 30 }, { NOTE_C5, 40 }, { NOTE_D5, 50 }, { NOTE_E5, 70 }, { NOTE_REST, 0 } };

static Note MELODY_REGEN_OFF[] = { { NOTE_E5, 70 }, { NOTE_D5, 50 }, { NOTE_C5, 40 }, { NOTE_B4, 30 }, { NOTE_A4, 30 }, { NOTE_G4, 30 },
                                   { NOTE_F4, 30 }, { NOTE_E4, 30 }, { NOTE_D4, 30 }, { NOTE_C4, 30 }, { NOTE_REST, 0 } };

static Note MELODY_CC_ENABLE[] = { { NOTE_C5, 200 }, { NOTE_REST, 100 }, { NOTE_G5, 200 }, { NOTE_REST, 0 } };
static Note MELODY_CC_DISABLE[] = { { NOTE_G5, 200 }, { NOTE_REST, 100 }, { NOTE_C5, 200 }, { NOTE_REST, 0 } };

static Note MELODY_CC_UP[] = { { NOTE_A4, 50 }, { NOTE_REST, 0 } };
static Note MELODY_CC_DOWN[] = { { NOTE_G3, 50 }, { NOTE_REST, 0 } };

static bool turn_sig_state = true;

static SoftTimer s_beep_timer = { 0U };
static SoftTimer s_melody_timer = { 0U };
static SoftTimer s_signal_timer = { 0U };

/* Melody playback state */
static const Note *s_current_melody = NULL;
static uint16_t s_melody_index = 0;
static bool s_melody_playing = false;

static uint16_t s_freq_to_period_us(NoteFrequency freq) {
  if (freq == NOTE_REST || freq == 0U) {
    return 0U;
  }
  return (1000000U / freq);
}

static StatusCode toggle_carrier() {  // turns the carrier signal on or off
  if (s_is_buzzer_on) {
    if (pwm_counter == PWM_LOW_DIV_AMT) {
      status_ok_or_return(pwm_set_dc(BUZZER_TIMER, BUZZER_DUTY, BUZZER_CHANNEL, false));
      s_is_buzzer_on = false;
      pwm_counter = 0;
    }
  } else {
    if (pwm_counter == PWM_HIGH_DIV_AMT) {
      status_ok_or_return(pwm_set_dc(BUZZER_TIMER, 0U, BUZZER_CHANNEL, false));
      s_is_buzzer_on = true;
      pwm_counter = 0;
    }
  }

  pwm_counter++;
  return STATUS_CODE_OK;
}

static uint16_t get_arr(uint16_t modulation_frequency) {
  uint16_t timer_frequency = 2 * modulation_frequency;
  uint32_t timer_clock = 80000000;  // 80MHz
  uint16_t arr = (uint16_t)((timer_clock / ((prescaler + 1) * timer_frequency)) - 1) / PWM_DIV;

  return arr;
}

static StatusCode hal_timer_init(uint16_t modulation_frequency) {
  __HAL_RCC_TIM6_CLK_ENABLE();

  uint16_t arr = get_arr(modulation_frequency);

  h_timer.Instance = TIM6;
  h_timer.Init.Prescaler = prescaler;
  h_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
  h_timer.Init.Period = arr;  // This sets the frequency
  h_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  h_timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

  #if (BUZZER_DEBUG == 1)
  LOG_DEBUG("Attempting to initialize HAL timer\r\n");
  delay_ms(10U);
  #endif

  if (HAL_TIM_Base_Init(&h_timer) != HAL_OK) {
    #if (BUZZER_DEBUG == 1)
    LOG_DEBUG("Error initalizing HAL timer!\r\n");
    delay_ms(10U);
    #endif
    return STATUS_CODE_INTERNAL_ERROR;
  }
  #if (BUZZER_DEBUG == 1)
  LOG_DEBUG("Finished base init\r\n");
  delay_ms(10U);
  #endif
  HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);

  return STATUS_CODE_OK;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *h_timer) {
  if (h_timer->Instance == TIM6 && !isNoteRest) {
    toggle_carrier();
  }
}

void TIM6_DAC_IRQHandler(void) {
  HAL_TIM_IRQHandler(&h_timer);
}

static StatusCode s_play_note_modulated(NoteFrequency freq) {
  if (freq == NOTE_REST || freq == 0U) {
    /* Stop PWM for rest */
    isNoteRest = true;
    status_ok_or_return(pwm_set_dc(BUZZER_TIMER, 0U, BUZZER_CHANNEL, false));
    pwm_counter = 0;
  } else {
    isNoteRest = false;
    uint16_t arr = get_arr(freq);

    #if (BUZZER_DEBUG == 1)
    LOG_DEBUG("Starting to play note of frequency: %d | ARR: %u\r\n", freq, arr);
    delay_ms(10U);
    #endif
    __HAL_TIM_SET_AUTORELOAD(&h_timer, arr);
    __HAL_TIM_SET_COUNTER(&h_timer, 0);
  }

  return STATUS_CODE_OK;
}

static StatusCode s_play_note(NoteFrequency freq) {
  if (freq == NOTE_REST || freq == 0U) {
    /* Stop PWM for rest */
    status_ok_or_return(pwm_set_dc(BUZZER_TIMER, 0U, BUZZER_CHANNEL, false));
  } else {
    uint16_t period_us = s_freq_to_period_us(freq);

    /* Reinitialize PWM with new period */
    status_ok_or_return(pwm_init(BUZZER_TIMER, period_us));
    status_ok_or_return(pwm_set_dc(BUZZER_TIMER, BUZZER_DUTY, BUZZER_CHANNEL, false));
  }

  return STATUS_CODE_OK;
}

static void s_melody_callback(SoftTimerId id) {
  (void)id;

  if (!s_melody_playing || s_current_melody == NULL) {
    pwm_set_dc(BUZZER_TIMER, 0U, BUZZER_CHANNEL, false);
    s_melody_playing = false;
    return;
  }

  /* Check if we've reached the end of the melody (REST with 0 duration) */
  if (s_current_melody[s_melody_index].frequency == NOTE_REST && s_current_melody[s_melody_index].duration_ms == 0) {
    pwm_set_dc(BUZZER_TIMER, 0U, BUZZER_CHANNEL, false);
    s_melody_playing = false;
    return;
  }

  /* Play current note */
  Note current_note = s_current_melody[s_melody_index];
  s_play_note(current_note.frequency);

  /* Move to next note */
  s_melody_index++;

  /* Schedule next note using melody timer */
  if (current_note.duration_ms > 0) {
    software_timer_init_and_start(current_note.duration_ms, s_melody_callback, &s_melody_timer);
  }
}

static void s_melody_modulated_callback(SoftTimerId id) {
  (void)id;

  if (!s_melody_playing || s_current_melody == NULL) {
    pwm_set_dc(BUZZER_TIMER, 0U, BUZZER_CHANNEL, false);
    s_melody_playing = false;
    return;
  }

  /* Check if we've reached the end of the melody (REST with 0 duration) */
  if (s_current_melody[s_melody_index].frequency == NOTE_REST && s_current_melody[s_melody_index].duration_ms == 0) {
    pwm_set_dc(BUZZER_TIMER, 0U, BUZZER_CHANNEL, false);
    s_melody_playing = false;
    return;
  }

  /* Play current note */
  Note current_note = s_current_melody[s_melody_index];
  s_play_note_modulated(current_note.frequency);

  /* Move to next note */
  s_melody_index++;

  /* Schedule next note using melody timer */
  if (current_note.duration_ms > 0) {
    software_timer_init_and_start(current_note.duration_ms, s_melody_modulated_callback, &s_melody_timer);
  }
}

static void s_beep_callback(SoftTimerId id) {
  (void)id;
  pwm_set_dc(BUZZER_TIMER, 0U, BUZZER_CHANNEL, false);
}

static void s_blink_signal_timer_callback(SoftTimerId timer_id) {
  if (turn_sig_state) {
    buzzer_play_melody(TURN_SIGNAL_CLICK_LOW);
    turn_sig_state = false;
  } else {
    buzzer_play_melody(TURN_SIGNAL_CLICK_HIGH);
    turn_sig_state = true;
  }

  software_timer_reset(&s_signal_timer);
}

StatusCode buzzer_init(BuzzerSettings settings) {
  status_ok_or_return(gpio_init_pin_af(&s_buzzer_pwm_pin, GPIO_ALTFN_PUSH_PULL, BUZZER_GPIO_ALTFN));
  status_ok_or_return(pwm_init(BUZZER_TIMER, s_freq_to_period_us(settings.amplitude_modulation_enabled ? CARRIER_FREQUENCY_KHZ * 1000 : NOTE_A4)));
  status_ok_or_return(software_timer_init(BUZZER_BEEP_DURATION_MS, s_beep_callback, &s_beep_timer));
  status_ok_or_return(software_timer_init(GLOBAL_SIGNAL_LIGHTS_BLINK_PERIOD_MS, s_blink_signal_timer_callback, &s_signal_timer));
  
  amplitude_modulation_enabled = settings.amplitude_modulation_enabled;

  if (settings.amplitude_modulation_enabled) {
    #if (BUZZER_DEBUG == 1)
    LOG_DEBUG("Attempting to initialize HAL timer...\r\n");
    delay_ms(10);
    #endif
    status_ok_or_return(hal_timer_init(NOTE_A4));
    
    #if (BUZZER_DEBUG == 1)
    LOG_DEBUG("HAL timer initialized...\r\n");
    delay_ms(10);
    #endif

    if (HAL_TIM_Base_Start_IT(&h_timer) != HAL_OK) {
      return STATUS_CODE_INTERNAL_ERROR;
    }

    #if (BUZZER_DEBUG == 1)
    LOG_DEBUG("finished start IT\r\n");
    delay_ms(10);
    #endif
  }

  return STATUS_CODE_OK;
}

StatusCode buzzer_beep(void) {
  /* Stop any playing melody */
  if (s_melody_playing) {
    s_melody_playing = false;
    if (s_melody_timer.id != NULL && software_timer_inuse(&s_melody_timer)) {
      software_timer_cancel(&s_melody_timer);
    }
  }

  if (software_timer_inuse(&s_beep_timer)) {
    /* Extend the beep by resetting the timer */
    status_ok_or_return(software_timer_reset(&s_beep_timer));
  } else {
    /* Play single beep at 3.5kHz */
    status_ok_or_return(pwm_init(BUZZER_TIMER, 1000000U / 3500U));
    status_ok_or_return(pwm_set_dc(BUZZER_TIMER, BUZZER_DUTY, BUZZER_CHANNEL, false));
    status_ok_or_return(software_timer_start(&s_beep_timer));
  }

  return STATUS_CODE_OK;
}

StatusCode buzzer_play_melody(Note *melody) {
  if (melody == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Stop current playback if any */
  s_melody_playing = false;

  /* Return error if any active timers */
  if (s_beep_timer.id != NULL && software_timer_inuse(&s_beep_timer)) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  if (s_melody_timer.id != NULL && software_timer_inuse(&s_melody_timer)) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  pwm_set_dc(BUZZER_TIMER, 0U, BUZZER_CHANNEL, false);

  /* Start new melody */
  s_current_melody = melody;
  s_melody_index = 0;
  s_melody_playing = true;

  /* Play first note */
  s_melody_callback(s_melody_timer.id);

  return STATUS_CODE_OK;
}

StatusCode buzzer_play_melody_modulated(Note *melody) {
  if (!amplitude_modulation_enabled) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (melody == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Stop current playback if any */
  s_melody_playing = false;

  /* Return error if any active timers */
  if (s_beep_timer.id != NULL && software_timer_inuse(&s_beep_timer)) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  if (s_melody_timer.id != NULL && software_timer_inuse(&s_melody_timer)) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  pwm_set_dc(BUZZER_TIMER, BUZZER_DUTY, BUZZER_CHANNEL, false);

  /* Start new melody */
  s_current_melody = melody;
  s_melody_index = 0;
  s_melody_playing = true;

  /* Play first note */
  s_melody_modulated_callback(s_melody_timer.id);

  return STATUS_CODE_OK;
}

StatusCode buzzer_play_startup(void) {
  return buzzer_play_melody(MELODY_STARTUP);
}

StatusCode buzzer_play_error(void) {
  return buzzer_play_melody(MELODY_ERROR);
}

StatusCode buzzer_play_success(void) {
  return buzzer_play_melody(MELODY_SUCCESS);
}

StatusCode buzzer_play_drive(void) {
  return buzzer_play_melody(MELODY_DRIVE_STATE);
}

StatusCode buzzer_play_neutral(void) {
  return buzzer_play_melody(MELODY_NEUTRAL_STATE);
}

StatusCode buzzer_play_reverse(void) {
  return buzzer_play_melody(MELODY_REVERSE_STATE);
}

StatusCode buzzer_play_invalid(void) {
  return buzzer_play_melody(MELODY_INVALID_STATE);
}

StatusCode buzzer_play_regen_on(void) {
  return buzzer_play_melody(MELODY_REGEN_ON);
}

StatusCode buzzer_play_regen_off(void) {
  return buzzer_play_melody(MELODY_REGEN_OFF);
}
StatusCode buzzer_play_cruise_control_enable(void) {
  return buzzer_play_melody(MELODY_CC_ENABLE);
}
StatusCode buzzer_play_cruise_control_disable(void) {
  return buzzer_play_melody(MELODY_CC_DISABLE);
}
StatusCode buzzer_play_cruise_control_up(void) {
  return buzzer_play_melody(MELODY_CC_UP);
}
StatusCode buzzer_play_cruise_control_down(void) {
  return buzzer_play_melody(MELODY_CC_DOWN);
}

StatusCode buzzer_start_turn_signal(void) {
  if (!software_timer_inuse(&s_signal_timer)) {
    software_timer_start(&s_signal_timer);
  }

  return STATUS_CODE_OK;
}

StatusCode buzzer_stop_turn_signal(void) {
  if (software_timer_inuse(&s_signal_timer)) {
    software_timer_cancel(&s_signal_timer);
    turn_sig_state = true;
  }
  return STATUS_CODE_OK;
}

StatusCode buzzer_stop(void) {
  s_melody_playing = false;

  /* Cancel both timers */
  if (s_beep_timer.id != NULL && software_timer_inuse(&s_beep_timer)) {
    software_timer_cancel(&s_beep_timer);
  }
  if (s_melody_timer.id != NULL && software_timer_inuse(&s_melody_timer)) {
    software_timer_cancel(&s_melody_timer);
  }

  pwm_set_dc(BUZZER_TIMER, 0U, BUZZER_CHANNEL, false);
  return STATUS_CODE_OK;
}

bool buzzer_is_playing(void) {
  return s_melody_playing;
}
