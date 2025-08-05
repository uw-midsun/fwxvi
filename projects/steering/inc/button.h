#pragma once

/************************************************************************************************
 * @file    button.h
 *
 * @brief   Header file for managing an individual GPIO-connected button with debounce and edge callbacks.
 *
 * @date    2025-07-28
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */

#include "gpio.h"

/* Intra-component Headers */

/**
 * @defgroup BUTTON
 * @brief Driver for an individual GPIO-connected button with debounce and edge-triggered callbacks.
 * @{
 */

/**
 * @brief Button states representing current input
 */

typedef enum { BUTTON_IDLE, BUTTON_PRESSED } ButtonState;

/**
 * @brief Button Callbacks
 */

typedef struct {
  void (*rising_edge_cb)(Button *button);
  void (*falling_edge_cb)(Button *button);
} ButtonCallbacks;

/**
 * @brief Button Config
 */

typedef struct {
  GpioAddress gpio;
  bool active_low;
  uint8_t debounce_ms;
  ButtonCallbacks callbacks;
} ButtonConfig;

/**
 * @brief Button Class
 */

typedef struct {
  ButtonConfig *config;
  uint8_t last_raw;
  uint8_t counter;
  ButtonState state;
} Button;

/**
 * @brief Initialise a button
 *
 * @param button The button
 * @param config Current button configuration
 */

void button_init(Button *button, ButtonConfig *config);

/**
 * @brief Update the Button state to the requested state
 *
 * @param button The button
 * @param state Current button state
 */

void button_update(Button *button, GpioState state);
