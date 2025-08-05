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

/**
 * @brief Initialize the button manager
 *
 * @param manager Pointer to the ButtonManager instance
 */
void button_manager_init(ButtonManager *manager, ButtonConfig *configs, uint8_t num_buttons) {
    manager->num_buttons = num_buttons;
    for (uint8_t i = 0; i < num_buttons; i++) {
        button_init(&manager->buttons[i], &configs[i]);
    }
}

/**
 * @brief Update the button manager
 *
 * @param manager Pointer to the ButtonManager instance
 */
void button_manager_update(ButtonManager *manager) {
    for (uint8_t i = 0; i < manager->num_buttons; i++) {
        button_update(&manager->buttons[i], gpio_get_state(&manager->buttons[i].config->gpio));
    }
}

