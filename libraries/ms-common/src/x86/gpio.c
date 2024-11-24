/************************************************************************************************
 * gpio.c
 *
 * GPIO Library Source Code
 *
 * Created: 2024-11-02
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "gpio.h"
#include "gpio_mcu.h"

static GpioMode s_gpio_pin_modes[GPIO_TOTAL_PINS];
static uint8_t s_gpio_pin_state[GPIO_TOTAL_PINS];
static GpioMode s_gpio_alt_functions[GPIO_TOTAL_PINS];

StatusCode gpio_init(void) {
  for (uint32_t k = 0; k < GPIO_TOTAL_PINS; ++k) {
    s_gpio_pin_state[k] = GPIO_STATE_LOW;
    s_gpio_alt_functions[k] = GPIO_ALT_NONE;
  }
  return STATUS_CODE_OK;
}

StatusCode gpio_init_pin(const GpioAddress *address, const GpioMode pin_mode,
                         GpioState init_state) {
  taskENTER_CRITICAL();

  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT ||
      pin_mode >= NUM_GPIO_MODES) {
    taskEXIT_CRITICAL();
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint32_t index = address -> port * (uint32_t)GPIO_PINS_PER_PORT + address -> pin;

  s_gpio_pin_modes[index] = pin_mode;
  s_gpio_pin_state[index] = init_state;
  s_gpio_alt_functions[index] = GPIO_ALT_NONE;
  taskEXIT_CRITICAL();

  return STATUS_CODE_OK;
}

StatusCode gpio_init_pin_af(const GpioAddress *address, const GpioMode pin_mode,
                            GpioAlternateFunctions alt_func) {
  taskENTER_CRITICAL();
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT ||
      pin_mode >= NUM_GPIO_MODES) {
    taskEXIT_CRITICAL();
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint32_t index = address -> port * (uint32_t)GPIO_PINS_PER_PORT + address -> pin;

  s_gpio_pin_modes[index] = pin_mode;
  s_gpio_alt_functions[index] = alt_func;

  #ifdef DEBUG
    LOG_DEBUG("Mode of alternate function pin -> %d /nMode of GPIO pin -> %d",  altfunc, pin_mode);
  #endif

  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}

StatusCode gpio_set_state(const GpioAddress *address, GpioState state) {
   if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    taskEXIT_CRITICAL();
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint32_t index = address -> port * (uint32_t)GPIO_PINS_PER_PORT + address -> pin;

  GpioMode mode = s_gpio_pin_modes[index];
   if (mode != GPIO_OUTPUT_OPEN_DRAIN && mode != GPIO_OUTPUT_PUSH_PULL) {
    LOG_WARN("Attempting to set an input pin, check your configuration");
    taskEXIT_CRITICAL();
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_gpio_pin_state[index] = state;

  #ifdef DEBUG
    LOG_DEBUG("State of GPIO pin (port %d, pin %d) -> %d", address->port, address->pin, state);
  #endif

  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}

StatusCode gpio_toggle_state(const GpioAddress *address) {
  taskENTER_CRITICAL();
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
  taskEXIT_CRITICAL();
  return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint32_t index = address -> port * (uint32_t)GPIO_PINS_PER_PORT + address -> pin;

  if (s_gpio_pin_state[index] == GPIO_STATE_LOW) {
    s_gpio_pin_state[index] = GPIO_STATE_HIGH;
  } else {
    s_gpio_pin_state[index] = GPIO_STATE_LOW;
  }

  #ifdef DEBUG
    LOG_DEBUG("Toggled state of GPIO pin (port %d, pin %d)", address->port, address->pin);
  #endif

  taskEXIT_CRITICAL();
  return STATUS_CODE_OK;
}

GpioState gpio_get_state(const GpioAddress *address) {
  taskENTER_CRITCAL();
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    taskEXIT_CRITICAL();
    return status_code(STATUS_CODE_INVALID_ARGS);

  if (address->port < NUM_GPIO_PORTS || address->pin < GPIO_PINS_PER_PORT) {
    uint32_t index = address -> port * (uint32_t)GPIO_PINS_PER_PORT + address -> pin;
    taskEXIT_CRITICAL();
    return s_gpio_pin_state[index];
  }
  taskEXIT_CRITICAL();
  return GPIO_STATE_LOW;
}
