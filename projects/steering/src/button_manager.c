/************************************************************************************************
 * @file    button_manager.c
 *
 * @brief   Source file for managing and polling multiple buttons on the steering board, including
 *          drive modes, hazards, turn signals, cruise control, and regen.
 *
 * @date    2025-07-28
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>

/* Inter-component Headers */
#include "gpio.h"
#include "log.h"

/* Intra-component Headers */
#include "button_led_manager.h"
#include "button_manager.h"
#include "buzzer.h"
#include "cruise_control.h"
#include "drive_state_manager.h"
#include "global_enums.h"
#include "light_signal_manager.h"
#include "party_mode.h"
#include "steering.h"
#include "steering_hw_defs.h"
#include "steering_setters.h"

static SteeringStorage *steering_storage;

static ButtonManager s_button_manager = { 0U };

static LEDPixels rgb_led_colors[NUM_STEERING_BUTTONS] = {
  [STEERING_BUTTON_HAZARDS] = BUTTON_LED_MANAGER_COLOR_RED,

  [STEERING_BUTTON_DRIVE] = BUTTON_LED_MANAGER_COLOR_GREEN,
  [STEERING_BUTTON_NEUTRAL] = BUTTON_LED_MANAGER_COLOR_WHITE,
  [STEERING_BUTTON_REVERSE] = BUTTON_LED_MANAGER_COLOR_BLUE,

  [STEERING_BUTTON_REGEN] = BUTTON_LED_MANAGER_COLOR_CYAN,

  [STEERING_BUTTON_LEFT_LIGHT] = BUTTON_LED_MANAGER_COLOR_YELLOW,
  [STEERING_BUTTON_RIGHT_LIGHT] = BUTTON_LED_MANAGER_COLOR_YELLOW,

  [STEERING_BUTTON_HORN] = BUTTON_LED_MANAGER_COLOR_ORANGE,

  [STEERING_BUTTON_CRUISE_CONTROL_UP] = BUTTON_LED_MANAGER_COLOR_PURPLE,
  [STEERING_BUTTON_CRUISE_CONTROL_DOWN] = BUTTON_LED_MANAGER_COLOR_PINK,
};

/************************************************************************************************
 * Left turn button handlers
 ************************************************************************************************/

static SteeringLightState light_state = STEERING_LIGHTS_OFF_STATE;

static void left_turn_btn_falling_edge_cb(Button *button) {
  // if (party_mode_active() == false) {
  //   buzzer_play_success();
  // }

  lights_signal_manager_request(LIGHTS_SIGNAL_STATE_LEFT);

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - LeftTurn Falling edge callback\r\n");
#endif

  if (light_state != STEERING_LIGHTS_LEFT_STATE && light_state != STEERING_LIGHTS_HAZARD_STATE) {
    buzzer_start_turn_signal();
    set_steering_buttons_lights(STEERING_LIGHTS_LEFT_STATE);
    light_state = STEERING_LIGHTS_LEFT_STATE;
  } else if (light_state != STEERING_LIGHTS_HAZARD_STATE) {
    buzzer_stop_turn_signal();
    set_steering_buttons_lights(STEERING_LIGHTS_OFF_STATE);
    light_state = STEERING_LIGHTS_OFF_STATE;
  }
}

static void left_turn_btn_rising_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - LeftTurn Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Right turn button handlers
 ************************************************************************************************/

static void right_turn_btn_falling_edge_cb(Button *button) {
  // if (party_mode_active() == false) {
  //   buzzer_play_success();
  // }

  lights_signal_manager_request(LIGHTS_SIGNAL_STATE_RIGHT);

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - RightTurn Falling edge callback\r\n");
#endif

  if (light_state != STEERING_LIGHTS_RIGHT_STATE && light_state != STEERING_LIGHTS_HAZARD_STATE) {
    buzzer_start_turn_signal();
    set_steering_buttons_lights(STEERING_LIGHTS_RIGHT_STATE);
    light_state = STEERING_LIGHTS_RIGHT_STATE;
  } else if (light_state != STEERING_LIGHTS_HAZARD_STATE) {
    buzzer_stop_turn_signal();
    set_steering_buttons_lights(STEERING_LIGHTS_OFF_STATE);
    light_state = STEERING_LIGHTS_OFF_STATE;
  }
}

static void right_turn_btn_rising_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - RightTurn Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Hazards button handlers
 ************************************************************************************************/

static void hazards_btn_falling_edge_cb(Button *button) {
  // if (party_mode_active() == false) {
  //   buzzer_play_success();
  // }

  lights_signal_manager_request(LIGHTS_SIGNAL_STATE_HAZARD);
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Hazards Falling edge callback\r\n");
#endif

  if (light_state != STEERING_LIGHTS_HAZARD_STATE) {
    buzzer_start_turn_signal();
    set_steering_buttons_lights(STEERING_LIGHTS_HAZARD_STATE);
    light_state = STEERING_LIGHTS_HAZARD_STATE;
  } else {
    buzzer_stop_turn_signal();
    set_steering_buttons_lights(STEERING_LIGHTS_OFF_STATE);
    light_state = STEERING_LIGHTS_OFF_STATE;
  }
}

static void hazards_btn_rising_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Hazards Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Drive button handlers
 ************************************************************************************************/

static void drive_btn_falling_edge_cb(Button *button) {
  if (party_mode_active() == false) {
    buzzer_play_success();
  }

  drive_state_manager_request(DRIVE_STATE_DRIVE);

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Drive Falling edge callback\r\n");
#endif
  set_steering_buttons_drive_state(VEHICLE_DRIVE_STATE_DRIVE);
}

static void drive_btn_rising_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Drive Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Reverse button handlers
 ************************************************************************************************/

static void reverse_btn_falling_edge_cb(Button *button) {
  if (party_mode_active() == false) {
    buzzer_play_success();
  }

  drive_state_manager_request(DRIVE_STATE_REVERSE);

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Reverse Falling edge callback\r\n");
#endif

  set_steering_buttons_drive_state(VEHICLE_DRIVE_STATE_REVERSE);
}

static void reverse_btn_rising_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Reverse Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Neutral button handlers
 ************************************************************************************************/

static void neutral_btn_falling_edge_cb(Button *button) {
  if (party_mode_active() == false) {
    buzzer_play_success();
  }

  drive_state_manager_request(DRIVE_STATE_NEUTRAL);

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Neutral Falling edge callback\r\n");
#endif

  set_steering_buttons_drive_state(VEHICLE_DRIVE_STATE_NEUTRAL);
}

static void neutral_btn_rising_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Neutral Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Horn button handlers
 ************************************************************************************************/

static void horn_btn_falling_edge_cb(Button *button) {
  if (party_mode_active() == false) {
    buzzer_play_success();
  }

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Horn Falling edge callback\r\n");
#endif

  set_steering_buttons_horn_enabled(true);
}

static void horn_btn_rising_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Horn Rising edge callback\r\n");
#endif

  set_steering_buttons_horn_enabled(false);
}

/************************************************************************************************
 * Regen button handlers
 ************************************************************************************************/

static void regen_btn_falling_edge_cb(Button *button) {
  if (party_mode_active() == false) {
    buzzer_play_success();
  }

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Regen Falling edge callback\r\n");
#endif
}

static void regen_btn_rising_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Regen Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Cruise control up button handlers
 ************************************************************************************************/

static void cruise_control_up_btn_falling_edge_cb(Button *button) {
  if (party_mode_active() == false) {
    buzzer_play_success();
  }

  cruise_control_up_handler();

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - CC up Falling edge callback\r\n");
#endif
}

static void cruise_control_up_btn_rising_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - CC up Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Cruise control down button handlers
 ************************************************************************************************/

static void cruise_control_down_btn_falling_edge_cb(Button *button) {
  if (party_mode_active() == false) {
    buzzer_play_success();
  }

  cruise_control_down_handler();

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - CC down Falling edge callback\r\n");
#endif
}

static void cruise_control_down_btn_rising_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - CC down Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Regen button handlers
 ************************************************************************************************/

static void regen_btn_falling_edge_cb(Button *button) {
  if (party_mode_active() == false) {
    buzzer_play_success();
  }

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Regen Falling edge callback\r\n");
#endif
}

static void regen_btn_rising_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Regen Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Cruise control up button handlers
 ************************************************************************************************/

static void cruise_control_up_btn_falling_edge_cb(Button *button) {
  if (party_mode_active() == false) {
    buzzer_play_success();
  }

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - CC up Falling edge callback\r\n");
#endif
}

static void cruise_control_up_btn_rising_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - CC up Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Cruise control down button handlers
 ************************************************************************************************/

static void cruise_control_down_btn_falling_edge_cb(Button *button) {
  if (party_mode_active() == false) {
    buzzer_play_success();
  }

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - CC down Falling edge callback\r\n");
#endif
}

static void cruise_control_down_btn_rising_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - CC down Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Button configs
 ************************************************************************************************/

static ButtonConfig s_button_configs[NUM_STEERING_BUTTONS] = {
    [STEERING_BUTTON_LEFT_LIGHT] = {
        .active_low = true,
        .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
        .callbacks = {
            .falling_edge_cb = left_turn_btn_falling_edge_cb,
            .rising_edge_cb  = left_turn_btn_rising_edge_cb,
        },
        .gpio = STEERING_LEFT_TURN_BUTTON
    },

    [STEERING_BUTTON_RIGHT_LIGHT] = {
        .active_low = true,
        .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
        .callbacks = {
            .falling_edge_cb = right_turn_btn_falling_edge_cb,
            .rising_edge_cb  = right_turn_btn_rising_edge_cb,
        },
        .gpio = STEERING_RIGHT_TURN_BUTTON
    },

    [STEERING_BUTTON_HAZARDS] = {
        .active_low = true,
        .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
        .callbacks = {
            .falling_edge_cb = hazards_btn_falling_edge_cb,
            .rising_edge_cb  = hazards_btn_rising_edge_cb,
        },
        .gpio = STEERING_HAZARDS_BUTTON
    },

    [STEERING_BUTTON_DRIVE] = {
        .active_low = true,
        .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
        .callbacks = {
            .falling_edge_cb = drive_btn_falling_edge_cb,
            .rising_edge_cb  = drive_btn_rising_edge_cb,
        },
        .gpio = STEERING_DRIVE_BUTTON
    },

    [STEERING_BUTTON_REVERSE] = {
        .active_low = true,
        .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
        .callbacks = {
            .falling_edge_cb = reverse_btn_falling_edge_cb,
            .rising_edge_cb  = reverse_btn_rising_edge_cb,
        },
        .gpio = STEERING_REVERSE_BUTTON
    },

    [STEERING_BUTTON_NEUTRAL] = {
        .active_low = true,
        .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
        .callbacks = {
            .falling_edge_cb = neutral_btn_falling_edge_cb,
            .rising_edge_cb  = neutral_btn_rising_edge_cb,
        },
        .gpio = STEERING_NEUTRAL_BUTTON
    },

    [STEERING_BUTTON_HORN] = {
        .active_low = true,
        .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
        .callbacks = {
            .falling_edge_cb = horn_btn_falling_edge_cb,
            .rising_edge_cb  = horn_btn_rising_edge_cb,
        },
        .gpio = STEERING_HORN_BUTTON
    },

    [STEERING_BUTTON_REGEN] = {
      .active_low = true,
      .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
      .callbacks = {
          .falling_edge_cb = regen_btn_falling_edge_cb,
          .rising_edge_cb  = regen_btn_rising_edge_cb,
      },
      .gpio = STEERING_REGEN_BUTTON
    },

    [STEERING_BUTTON_CRUISE_CONTROL_UP] = {
      .active_low = true,
      .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
      .callbacks = {
          .falling_edge_cb = cruise_control_up_btn_falling_edge_cb,
          .rising_edge_cb  = cruise_control_up_btn_rising_edge_cb,
      },
      .gpio = STEERING_CC_UP_BUTTON
    },

    [STEERING_BUTTON_CRUISE_CONTROL_DOWN] = {
      .active_low = true,
      .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
      .callbacks = {
          .falling_edge_cb = cruise_control_down_btn_falling_edge_cb,
          .rising_edge_cb  = cruise_control_down_btn_rising_edge_cb,
      },
      .gpio = STEERING_CC_DOWN_BUTTON
    },
};

/************************************************************************************************
 * Public functions
 ************************************************************************************************/

StatusCode button_manager_init(SteeringStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  steering_storage = storage;
  steering_storage->button_manager = &s_button_manager;

  for (uint8_t i = 0U; i < NUM_STEERING_BUTTONS; i++) {
    status_ok_or_return(button_init(&steering_storage->button_manager->buttons[i], &s_button_configs[i]));
    status_ok_or_return(button_led_manager_set_color(i, rgb_led_colors[i]));
  }

  return STATUS_CODE_OK;
}

StatusCode button_manager_reset(void) {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  for (uint8_t i = 0; i < NUM_STEERING_BUTTONS; i++) {
    status_ok_or_return(button_led_manager_set_color(i, rgb_led_colors[i]));
  }

  return STATUS_CODE_OK;
}

StatusCode button_manager_update(void) {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  for (uint8_t i = 0; i < NUM_STEERING_BUTTONS; i++) {
    status_ok_or_return(button_update(&steering_storage->button_manager->buttons[i]));
  }

  return STATUS_CODE_OK;
}

StatusCode button_manager_led_enable(SteeringButtons button) {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  status_ok_or_return(button_led_manager_set_color(button, rgb_led_colors[button]));

  return STATUS_CODE_OK;
}

StatusCode button_manager_led_disable(SteeringButtons button) {
  if (steering_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  status_ok_or_return(button_led_manager_set_color(button, (LEDPixels)BUTTON_LED_MANAGER_COLOR_OFF));

  return STATUS_CODE_OK;
}
