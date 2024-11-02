#pragma once

/************************************************************************************************
 * gpio.h
 *
 * GPIO Library
 *
 * Created: 2024-10-27
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <stdint.h>

/* Inter-component Headers */
#include "stm32l4xx_hal_gpio.h"

/* Intra-component Headers */
#include "gpio_mcu.h"
#include "status.h"

typedef struct GpioAddress {
  uint8_t port;
  uint8_t pin;
} GpioAddress;

// Available modes for the GPIO pins
// See Section 9.1.11 of stm32f10x reference manual for
// configurations needed for different peripherals
typedef enum {
  GPIO_ANALOG = 0,
  GPIO_INPUT_FLOATING,
  GPIO_INPUT_PULL_DOWN,
  GPIO_INPUT_PULL_UP,
  GPIO_OUTPUT_OPEN_DRAIN,
  GPIO_OUTPUT_PUSH_PULL,
  GPIO_ALFTN_OPEN_DRAIN,
  GPIO_ALTFN_PUSH_PULL,
  NUM_GPIO_MODES,
} GpioMode;

/**
 * @brief   Initializes GPIO globally by setting all pins to their default state
 * @details ONLY CALL ONCE or it will deinit all current settings. Change pin setting by calling
 *          gpio_init_pin.
 */
StatusCode gpio_init(void);

/**
 * @brief   Initializes a GPIO pin by address
 * @details GPIOs are configured to a specified mode, at the max refresh speed
 * 
 */
StatusCode gpio_init_pin(const GpioAddress *address, const GpioMode pin_mode, GpioState init_state);

// Set the pin state by address.
StatusCode gpio_set_state(const GpioAddress *address, GpioState state);

// Toggles the output state of the pin.
StatusCode gpio_toggle_state(const GpioAddress *address);

// Gets the value of the input register for a pin and assigns it to the state
// that is passed in.
StatusCode gpio_get_state(const GpioAddress *address, GpioState *input_state);
