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

/* Intra-component Headers */
#include "button_manager.h"

StatusCode button_manager_init(ButtonManager *manager, ButtonConfig *configs, uint8_t num_buttons) {
  if (manager == NULL || configs == NULL || num_buttons > BUTTON_MANAGER_MAX_BUTTONS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  manager->num_buttons = num_buttons;

  for (uint8_t i = 0U; i < num_buttons; i++) {
    status_ok_or_return(button_init(&manager->buttons[i], &configs[i]));
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
