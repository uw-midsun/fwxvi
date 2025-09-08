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
#include "button_manager.h"
#include "drive_state_manager.h"
#include "light_signal_manager.h"
#include "steering.h"
#include "steering_hw_defs.h"

/************************************************************************************************
 * Left turn button handlers
 ************************************************************************************************/

static void left_turn_btn_falling_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - LeftTurn Falling edge callback\r\n");
#endif
}

static void left_turn_btn_rising_edge_cb(Button *button) {
  lights_signal_manager_request(LIGHTS_SIGNAL_STATE_LEFT);

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - LeftTurn Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Right turn button handlers
 ************************************************************************************************/

static void right_turn_btn_falling_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - RightTurn Falling edge callback\r\n");
#endif
}

static void right_turn_btn_rising_edge_cb(Button *button) {
  lights_signal_manager_request(LIGHTS_SIGNAL_STATE_RIGHT);

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - RightTurn Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Hazards button handlers
 ************************************************************************************************/

static void hazards_btn_falling_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Hazards Falling edge callback\r\n");
#endif
}

static void hazards_btn_rising_edge_cb(Button *button) {
  lights_signal_manager_request(LIGHTS_SIGNAL_STATE_HAZARD);

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Hazards Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Drive button handlers
 ************************************************************************************************/

static void drive_btn_falling_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Drive Falling edge callback\r\n");
#endif
}

static void drive_btn_rising_edge_cb(Button *button) {
  drive_state_manager_request(DRIVE_STATE_DRIVE);

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Drive Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Reverse button handlers
 ************************************************************************************************/

static void reverse_btn_falling_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Reverse Falling edge callback\r\n");
#endif
}

static void reverse_btn_rising_edge_cb(Button *button) {
  drive_state_manager_request(DRIVE_STATE_REVERSE);

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Reverse Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Neutral button handlers
 ************************************************************************************************/

static void neutral_btn_falling_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Neutral Falling edge callback\r\n");
#endif
}

static void neutral_btn_rising_edge_cb(Button *button) {
  drive_state_manager_request(DRIVE_STATE_NEUTRAL);

#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Neutral Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Horn button handlers
 ************************************************************************************************/

static void horn_btn_falling_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Horn Falling edge callback\r\n");
#endif
}

static void horn_btn_rising_edge_cb(Button *button) {
#if (BUTTON_MANAGER_DEBUG)
  LOG_DEBUG("ButtonManager - Horn Rising edge callback\r\n");
#endif
}

/************************************************************************************************
 * Button configs
 ************************************************************************************************/

static ButtonConfig s_button_configs[NUM_STEERING_BUTTONS] = {
    [STEERING_BUTTON_LEFT_LIGHT] = {
        .active_low = false,
        .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
        .callbacks = {
            .falling_edge_cb = left_turn_btn_falling_edge_cb,
            .rising_edge_cb  = left_turn_btn_rising_edge_cb,
        },
        .gpio = STEERING_LEFT_TURN_BUTTON
    },

    [STEERING_BUTTON_RIGHT_LIGHT] = {
        .active_low = false,
        .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
        .callbacks = {
            .falling_edge_cb = right_turn_btn_falling_edge_cb,
            .rising_edge_cb  = right_turn_btn_rising_edge_cb,
        },
        .gpio = STEERING_RIGHT_TURN_BUTTON
    },

    [STEERING_BUTTON_HAZARDS] = {
        .active_low = false,
        .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
        .callbacks = {
            .falling_edge_cb = hazards_btn_falling_edge_cb,
            .rising_edge_cb  = hazards_btn_rising_edge_cb,
        },
        .gpio = STEERING_HAZARDS_BUTTON
    },

    [STEERING_BUTTON_DRIVE] = {
        .active_low = false,
        .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
        .callbacks = {
            .falling_edge_cb = drive_btn_falling_edge_cb,
            .rising_edge_cb  = drive_btn_rising_edge_cb,
        },
        .gpio = STEERING_DRIVE_BUTTON
    },

    [STEERING_BUTTON_REVERSE] = {
        .active_low = false,
        .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
        .callbacks = {
            .falling_edge_cb = reverse_btn_falling_edge_cb,
            .rising_edge_cb  = reverse_btn_rising_edge_cb,
        },
        .gpio = STEERING_REVERSE_BUTTON
    },

    [STEERING_BUTTON_NEUTRAL] = {
        .active_low = false,
        .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
        .callbacks = {
            .falling_edge_cb = neutral_btn_falling_edge_cb,
            .rising_edge_cb  = neutral_btn_rising_edge_cb,
        },
        .gpio = STEERING_NEUTRAL_BUTTON
    },

    [STEERING_BUTTON_HORN] = {
        .active_low = false,
        .debounce_ms = STEERING_BUTTON_DEBOUNCE_PERIOD_MS,
        .callbacks = {
            .falling_edge_cb = horn_btn_falling_edge_cb,
            .rising_edge_cb  = horn_btn_rising_edge_cb,
        },
        .gpio = STEERING_HORN_BUTTON
    },
};

/************************************************************************************************
 * Public functions
 ************************************************************************************************/

StatusCode button_manager_init(ButtonManager *manager) {
  if (manager == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  manager->num_buttons = NUM_STEERING_BUTTONS;

  for (uint8_t i = 0U; i < NUM_STEERING_BUTTONS; i++) {
    status_ok_or_return(button_init(&manager->buttons[i], &s_button_configs[i]));
  }

  return STATUS_CODE_OK;
}

StatusCode button_manager_update(ButtonManager *manager) {
  if (manager == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  for (uint8_t i = 0; i < manager->num_buttons; i++) {
    status_ok_or_return(button_update(&manager->buttons[i]));
  }

  return STATUS_CODE_OK;
}
