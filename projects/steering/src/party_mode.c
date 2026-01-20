/************************************************************************************************
 * @file    party_mode.c
 *
 * @brief   Source file for steering party mode
 *
 * @date    2025-11-11
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>

/* Inter-component Headers */
#include "gpio.h"
#include "status.h"

/* Intra-component Headers */
#include "button_led_manager.h"
#include "button_manager.h"
#include "buzzer.h"
#include "party_mode.h"

/* Using 150ms for the base eighth note length for a good tempo */
#define NOTE_BASE_MS 150U

/* Duration constants based on the 150ms base */
#define NOTE_EIGHTH (NOTE_BASE_MS)     /**< 150ms */
#define NOTE_QUARTER (NOTE_EIGHTH * 2) /**< 300ms */
#define NOTE_HALF (NOTE_EIGHTH * 4)    /**< 600ms */
#define NOTE_TRIPLET (NOTE_EIGHTH * 3) /**< 450ms */
#define NOTE_WHOLE (NOTE_EIGHTH * 8)   /**< 1200ms */

/* Rest (gap) after each note for separation (staccato feel) */
#define GAP_MS 30U
#define PLAY_TIME(duration) ((duration) - GAP_MS)

static bool party_mode = false;
static uint8_t party_hue = 0;
static bool toggle_triggered = false;

static SteeringStorage *steering_storage = NULL;
static LEDPixels saved_colors[NUM_STEERING_BUTTONS];
static bool saved_valid = false; 

static Note MELODY_MARIO[] = {
  // Phrase 1 (Loop 1)
  { NOTE_E5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_E5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_REST, NOTE_EIGHTH },
  { NOTE_E5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_REST, NOTE_EIGHTH },
  { NOTE_C5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_E5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },

  { NOTE_G5, PLAY_TIME(NOTE_QUARTER) },
  { NOTE_REST, GAP_MS },
  { NOTE_REST, NOTE_QUARTER },
  { NOTE_G4, PLAY_TIME(NOTE_QUARTER) },
  { NOTE_REST, GAP_MS },
  { NOTE_REST, NOTE_QUARTER },

  // Phrase 2 (Loop 1)
  { NOTE_C5, PLAY_TIME(NOTE_QUARTER) },
  { NOTE_REST, GAP_MS },
  { NOTE_REST, NOTE_EIGHTH },
  { NOTE_G4, PLAY_TIME(NOTE_QUARTER) },
  { NOTE_REST, GAP_MS },
  { NOTE_REST, NOTE_EIGHTH },
  { NOTE_E4, PLAY_TIME(NOTE_QUARTER) },
  { NOTE_REST, GAP_MS },
  { NOTE_REST, NOTE_EIGHTH },

  { NOTE_A4, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_B4, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_AS4, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_A4, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },

  // Phrase 3 (Loop 1)
  { NOTE_G4, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_E5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_G5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_A5, PLAY_TIME(NOTE_QUARTER) },
  { NOTE_REST, GAP_MS },
  { NOTE_F5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_G5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },

  { NOTE_REST, NOTE_EIGHTH },
  { NOTE_E5, PLAY_TIME(NOTE_QUARTER) },
  { NOTE_REST, GAP_MS },
  { NOTE_C5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_D5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_B4, PLAY_TIME(NOTE_QUARTER) },
  { NOTE_REST, GAP_MS },

  // --- FULL LOOP REPEAT ---

  // Phrase 1 (Loop 2)
  { NOTE_E5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_E5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_REST, NOTE_EIGHTH },
  { NOTE_E5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_REST, NOTE_EIGHTH },
  { NOTE_C5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_E5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },

  { NOTE_G5, PLAY_TIME(NOTE_QUARTER) },
  { NOTE_REST, GAP_MS },
  { NOTE_REST, NOTE_QUARTER },
  { NOTE_G4, PLAY_TIME(NOTE_QUARTER) },
  { NOTE_REST, GAP_MS },
  { NOTE_REST, NOTE_QUARTER },

  // Phrase 2 (Loop 2)
  { NOTE_C5, PLAY_TIME(NOTE_QUARTER) },
  { NOTE_REST, GAP_MS },
  { NOTE_REST, NOTE_EIGHTH },
  { NOTE_G4, PLAY_TIME(NOTE_QUARTER) },
  { NOTE_REST, GAP_MS },
  { NOTE_REST, NOTE_EIGHTH },
  { NOTE_E4, PLAY_TIME(NOTE_QUARTER) },
  { NOTE_REST, GAP_MS },
  { NOTE_REST, NOTE_EIGHTH },

  { NOTE_A4, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_B4, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_AS4, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_A4, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },

  // Phrase 3 (Loop 2)
  { NOTE_G4, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_E5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_G5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_A5, PLAY_TIME(NOTE_QUARTER) },
  { NOTE_REST, GAP_MS },
  { NOTE_F5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_G5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },

  { NOTE_REST, NOTE_EIGHTH },
  { NOTE_E5, PLAY_TIME(NOTE_QUARTER) },
  { NOTE_REST, GAP_MS },
  { NOTE_C5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_D5, PLAY_TIME(NOTE_EIGHTH) },
  { NOTE_REST, GAP_MS },
  { NOTE_G5, PLAY_TIME(NOTE_HALF) },
  { NOTE_REST, GAP_MS },

  { NOTE_REST, 0 }
};

static void party_mode_save_colors(void) {
  for (uint8_t i = 0; i < NUM_STEERING_BUTTONS; i++) {
    saved_colors[i] = steering_storage->button_led_manager->led_pixels[i];
  }
  saved_valid = true;
}

static void party_mode_restore_colors(void) {
  if (!saved_valid) {
    return;
  }
  for (uint8_t i = 0; i < NUM_STEERING_BUTTONS; i++) {
    button_led_manager_set_color(i, saved_colors[i]);
  }
  button_led_manager_update();
  saved_valid = false;
}

static LEDPixels hsv_to_rgb(uint8_t hue) {
  uint8_t region = hue / 43;
  uint8_t remainder = (hue - (region * 43)) * 6;

  uint8_t p = 0;
  uint8_t q = (255 * (255 - remainder)) / 255;
  uint8_t t = (255 * remainder) / 255;

  switch (region) {
    case 0:
      return ((LEDPixels){ 255, t, p });
    case 1:
      return ((LEDPixels){ q, 255, p });
    case 2:
      return ((LEDPixels){ p, 255, t });
    case 3:
      return ((LEDPixels){ p, q, 255 });
    case 4:
      return ((LEDPixels){ t, p, 255 });
    default:
      return ((LEDPixels){ 255, p, q });
  }
}

StatusCode party_mode_init(SteeringStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  steering_storage = storage;

  return STATUS_CODE_OK;
}

StatusCode party_mode_run(void) {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  bool both_pressed = (steering_storage->button_manager->buttons[STEERING_BUTTON_CRUISE_CONTROL_UP].state == BUTTON_PRESSED &&
                       steering_storage->button_manager->buttons[STEERING_BUTTON_CRUISE_CONTROL_DOWN].state == BUTTON_PRESSED);

  if (both_pressed && !toggle_triggered) {
    toggle_triggered = true;
    if (party_mode) {
      party_mode = false;
      party_mode_restore_colors();
      buzzer_stop();
    } else {
      party_mode_save_colors();
      party_mode = true;
      buzzer_stop();
    }
  } else if (!both_pressed) {
    toggle_triggered = false;
  }

  if (party_mode) {
    LEDPixels color = hsv_to_rgb(party_hue);

    for (uint8_t i = 0; i < NUM_STEERING_BUTTONS; i++) {
      status_ok_or_return(button_led_manager_set_color(i, color));
    }

    party_hue = (party_hue + 5U) % 255U;

    if (buzzer_is_playing() == false) {
      buzzer_play_melody(MELODY_MARIO);
    }
  }
  return STATUS_CODE_OK;
}

bool party_mode_active(void) {
  return party_mode;
}
