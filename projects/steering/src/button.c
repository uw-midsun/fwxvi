/************************************************************************************************
 * @file    button.c
 *
 * @brief   Source file for managing an individual GPIO-connected button with debounce and edge callbacks.
 *
 * @date    2025-07-28
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"

/* Intra-component Headers */
#include "button.h"

StatusCode button_init(Button *button, ButtonConfig *config) {
  if (button == NULL || config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  button->config = config;

  gpio_init_pin(&config->gpio, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  delay_ms(config->debounce_ms);

  GpioState startup_state = gpio_get_state(&config->gpio);

  bool is_pressed = (startup_state == GPIO_STATE_LOW && button->config->active_low) || (startup_state == GPIO_STATE_HIGH && !button->config->active_low);

  button->last_raw = is_pressed ? 0U : 1U;
  button->state = is_pressed ? BUTTON_PRESSED : BUTTON_IDLE;
  button->counter = 0U;

  return STATUS_CODE_OK;
}

StatusCode button_update(Button *button) {
  if (button == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  GpioState state = gpio_get_state(&button->config->gpio);

  bool is_pressed = (state == GPIO_STATE_LOW && button->config->active_low) || (state == GPIO_STATE_HIGH && !button->config->active_low);
  uint8_t current_raw = is_pressed ? 1U : 0U;

  if (current_raw != button->last_raw) {
    button->counter = 0U;
    button->last_raw = current_raw;
  } else if (button->counter < button->config->debounce_ms) {
    button->counter++;

    if (button->counter == button->config->debounce_ms) {
      ButtonState new_state = current_raw ? BUTTON_PRESSED : BUTTON_IDLE;

      if (new_state != button->state) {
        if (new_state == BUTTON_PRESSED) {
          if (button->config->callbacks.rising_edge_cb) {
            button->config->callbacks.rising_edge_cb(button);
          }
        } else {
          if (button->config->callbacks.falling_edge_cb) {
            button->config->callbacks.falling_edge_cb(button);
          }
        }

        button->state = new_state;
      }
    }
  }

  return STATUS_CODE_OK;
}
