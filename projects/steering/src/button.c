#pragma once

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

/* Inter-component Headers */

#include "gpio.h"

/* Intra-component Headers */

#include "button.h"

void button_init(Button *button, ButtonConfig *config){
    button->config = config;
    GpioState raw = gpio_get_state(&config->gpio);
    bool is_pressed = (raw == GPIO_STATE_LOW && button->config->active_low) ||
                      (raw == GPIO_STATE_HIGH && !button->config->active_low);
    button->last_raw = is_pressed ? 0 : 1;
    button->state = is_pressed ?  BUTTON_PRESSED : BUTTON_IDLE;
    button->counter = 0;
}

void button_update(Button* button, GpioState state) {
    
}

