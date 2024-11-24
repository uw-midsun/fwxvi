#pragma once

/************************************************************************************************
 * @file   gpio.h
 *
 * @brief  GPIO Library Header file
 *
 * @date   2024-11-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "gpio_mcu.h"
#include "status.h"

/**
 * @defgroup  GPIO
 * @brief     GPIO library
 * @{
 */

/**
 * @brief   Port and pin data
 */
typedef struct GpioAddress {
  GpioPort port;
  uint8_t pin;
} GpioAddress;

/**
 * @brief   Initializes GPIO globally by disabling JTAG and enabling all GPIO clocks
 * @details ONLY CALL ONCE or it will deinit all current settings. Change pin setting by calling
 *          gpio_init_pin
 * @return  STATUS_CODE_OK if intialization succeeded.
 */
StatusCode gpio_init(void);

/**
 * @brief   Initializes a GPIO pin by address
 * @details GPIOs are configured to a specified mode, at the max refresh speed
 * @param   address Pointer to the GPIO address
 * @param   pin_mode Pin configuration mode
 * @param   init_state Initial GPIO state for output pins
 * @return  STATUS_CODE_OK if pin initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode gpio_init_pin(const GpioAddress *address, const GpioMode pin_mode, GpioState init_state);

/**
 * @brief   Initializes an alternate function for a GPIO pin by address
 * @details This initialization should be used I2C, SPI, UART and timers
 * @param   address Pointer to the GPIO address
 * @param   pin_mode Pin configuration mode
 * @param   init_state Initial GPIO state for output pins
 * @return  STATUS_CODE_OK if pin initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode gpio_init_pin_af(const GpioAddress *address, const GpioMode pin_mode,
                            GpioAlternateFunctions alt_func);

/**
 * @brief   Sets the GPIO pin to a valid state
 * @param   address to the GPIO address
 * @param   state GPIO state can either be HIGH/LOW
 * @return  STATUS_CODE_OK if pin writing succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode gpio_set_state(const GpioAddress *address, GpioState state);

/**
 * @brief   Toggles the GPIO
 * @param   address to the GPIO address
 * @return  STATUS_CODE_OK if pin toggling succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode gpio_toggle_state(const GpioAddress *address);

/**
 * @brief   Toggles the GPIO
 * @param   address to the GPIO address
 * @return  STATUS_CODE_OK if pin reading succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
GpioState gpio_get_state(const GpioAddress *address);

/** @} */
