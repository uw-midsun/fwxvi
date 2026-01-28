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
#include "pwm.h"
#include "software_timer.h"

/* Intra-component Headers */
#include "buzzer.h"
#include "steering_hw_defs.h"

#define BUZZER_TIMER PWM_TIMER_4
#define BUZZER_CHANNEL PWM_CHANNEL_1
#define BUZZER_GPIO_ALTFN GPIO_ALT2_TIM4
#define BUZZER_DUTY 50U
#define BUZZER_BEEP_DURATION_MS 250U

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

static bool turn_sig_state = true;

static GpioAddress s_buzzer_pwm_pin = GPIO_STEERING_BUZZER_PWM_PIN;

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

StatusCode buzzer_init(void) {
  status_ok_or_return(gpio_init_pin_af(&s_buzzer_pwm_pin, GPIO_ALTFN_PUSH_PULL, BUZZER_GPIO_ALTFN));
  status_ok_or_return(pwm_init(BUZZER_TIMER, s_freq_to_period_us(NOTE_A4)));
  status_ok_or_return(software_timer_init(BUZZER_BEEP_DURATION_MS, s_beep_callback, &s_beep_timer));
  status_ok_or_return(software_timer_init(GLOBAL_SIGNAL_LIGHTS_BLINK_PERIOD_MS, s_blink_signal_timer_callback, &s_signal_timer));

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
