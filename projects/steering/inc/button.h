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
 * @brief    Driver for an individual GPIO-connected button with debounce and edge-triggered callbacks
 * @{
 */

/**
 * @brief Button states representing current input
 */
typedef enum {
  BUTTON_IDLE,    /**< Button is not pressed */
  BUTTON_PRESSED /**< Button is pressed */
} ButtonState;

/**
 * @brief Forward declaration for Button struct
 */
typedef struct Button Button;

/**
 * @brief Button Callbacks
 */
typedef struct {
  void (*rising_edge_cb)(Button *button);   /**< Called on rising edge (button press) */
  void (*falling_edge_cb)(Button *button);  /**< Called on falling edge (button release) */
} ButtonCallbacks;

/**
 * @brief Button configuration structure
 */
typedef struct {
  GpioAddress gpio;           /**< GPIO address for the button */
  bool active_low;            /**< True if button is active low */
  uint8_t debounce_ms;        /**< Debounce time in milliseconds */
  ButtonCallbacks callbacks;  /**< Edge callback functions */
} ButtonConfig;

/**
 * @brief Button instance structure
 */
typedef struct Button {
  ButtonConfig *config;   /**< Pointer to button configuration */
  uint8_t last_raw;       /**< Last raw GPIO value */
  uint8_t counter;        /**< Debounce counter */
  ButtonState state;      /**< Current debounced state */
} Button;

/**
 * @brief Initialize a button instance
 *
 * @param button Pointer to the Button instance to initialize
 * @param config Pointer to the ButtonConfig structure
 */
void button_init(Button *button, ButtonConfig *config);

/**
 * @brief Update the Button state based on the given GPIO state
 *
 * @param button Pointer to the Button instance
 * @param state  Current raw GPIO state
 */
void button_update(Button *button, GpioState state);

/** @} */
