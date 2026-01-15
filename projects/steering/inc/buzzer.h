#pragma once

/************************************************************************************************
 * @file    buzzer.h
 *
 * @brief   Header file for steering buzzer
 *
 * @date    2025-07-28
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "gpio.h"
#include "status.h"

/* Intra-component Headers */

/**
 * @defgroup steering
 * @brief    steering Firmware
 * @{
 */

/* Musical note frequencies in Hz */
typedef enum {
  NOTE_REST = 0,

  /* Octave 3 */
  NOTE_C3 = 131,
  NOTE_CS3 = 139, /* C# / Db */
  NOTE_D3 = 147,
  NOTE_DS3 = 156, /* D# / Eb */
  NOTE_E3 = 165,
  NOTE_F3 = 175,
  NOTE_FS3 = 185, /* F# / Gb */
  NOTE_G3 = 196,
  NOTE_GS3 = 208, /* G# / Ab */
  NOTE_A3 = 220,
  NOTE_AS3 = 233, /* A# / Bb */
  NOTE_B3 = 247,

  /* Octave 4 */
  NOTE_C4 = 262,
  NOTE_CS4 = 277, /* C# / Db */
  NOTE_D4 = 294,
  NOTE_DS4 = 311, /* D# / Eb */
  NOTE_E4 = 330,
  NOTE_F4 = 349,
  NOTE_FS4 = 370, /* F# / Gb */
  NOTE_G4 = 392,
  NOTE_GS4 = 415, /* G# / Ab */
  NOTE_A4 = 440,
  NOTE_AS4 = 466, /* A# / Bb */
  NOTE_B4 = 494,

  /* Octave 5 */
  NOTE_C5 = 523,
  NOTE_CS5 = 554, /* C# / Db */
  NOTE_D5 = 587,
  NOTE_DS5 = 622, /* D# / Eb */
  NOTE_E5 = 659,
  NOTE_F5 = 698,
  NOTE_FS5 = 740, /* F# / Gb */
  NOTE_G5 = 784,
  NOTE_GS5 = 831, /* G# / Ab */
  NOTE_A5 = 880,
  NOTE_AS5 = 932, /* A# / Bb */
  NOTE_B5 = 988,

  /* Octave 6 */
  NOTE_C6 = 1047,
  NOTE_CS6 = 1109, /* C# / Db */
  NOTE_D6 = 1175,
  NOTE_DS6 = 1245, /* D# / Eb */
  NOTE_E6 = 1319,
  NOTE_F6 = 1397,
  NOTE_FS6 = 1480, /* F# / Gb */
  NOTE_G6 = 1568,
  NOTE_GS6 = 1661, /* G# / Ab */
  NOTE_A6 = 1760,
  NOTE_AS6 = 1865, /* A# / Bb */
  NOTE_B6 = 1976,

  /* Octave 7 */
  NOTE_C7 = 2093,
  NOTE_CS7 = 2217, /* C# / Db */
  NOTE_D7 = 2349,
  NOTE_DS7 = 2489, /* D# / Eb */
  NOTE_E7 = 2637,
  NOTE_F7 = 2794,
  NOTE_FS7 = 2960, /* F# / Gb */
  NOTE_G7 = 3136,
  NOTE_GS7 = 3322, /* G# / Ab */
  NOTE_A7 = 3520,
  NOTE_AS7 = 3729, /* A# / Bb */
  NOTE_B7 = 3951,

  /* Octave 8 - Near 4kHz resonance */
  NOTE_C8 = 4186
} NoteFrequency;

/* Note structure for creating melodies */
typedef struct {
  NoteFrequency frequency;
  uint16_t duration_ms;
} Note;

/**
 * @brief   Initialize the buzzer
 * @return  STATUS_CODE_OK on success
 */
StatusCode buzzer_init(void);

/**
 * @brief   Play a simple beep at 3.5kHz
 * @return  STATUS_CODE_OK on success
 */
StatusCode buzzer_beep(void);

/**
 * @brief   Play a custom melody
 * @param   melody Array of Note structures, terminated by {NOTE_REST, 0}
 * @return  STATUS_CODE_OK on success
 */
StatusCode buzzer_play_melody(Note *melody);

/**
 * @brief   Play predefined startup melody
 * @return  STATUS_CODE_OK on success
 */
StatusCode buzzer_play_startup(void);

/**
 * @brief   Play predefined error melody
 * @return  STATUS_CODE_OK on success
 */
StatusCode buzzer_play_error(void);

/**
 * @brief   Play predefined success melody
 * @return  STATUS_CODE_OK on success
 */
StatusCode buzzer_play_success(void);

/**
 * @brief   Play turn signal click
 * @return  STATUS_CODE_OK on success
 */
StatusCode buzzer_start_turn_signal(void);

/**
 * @brief   Stop turn signal click
 * @return  STATUS_CODE_OK on success
 */
StatusCode buzzer_stop_turn_signal(void);

/**
 * @brief   Stop any currently playing sound
 * @return  STATUS_CODE_OK on success
 */
StatusCode buzzer_stop(void);

/**
 * @brief   Check if a melody is currently playing
 * @return  true if playing, false otherwise
 */
bool buzzer_is_playing(void);

/** @} */
