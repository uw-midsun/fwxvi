#pragma once

/************************************************************************************************
 * @file   gpio.h
 *
 * @brief  GPIO Library Header file
 *
 * @date   2024-11-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "status.h"

/**
 * @defgroup GPIO
 * @brief    GPIO library
 * @{
 */

#ifndef GPIO_PINS_PER_PORT
/** @brief  16 Pins per port */
#define GPIO_PINS_PER_PORT 16U
#endif

#ifndef GPIO_ADDRESS_OFFSET
/** @brief  Each GPIO Port is seperated in the MMIO by 1024 bytes */
#define GPIO_ADDRESS_OFFSET 0x400U
#endif

/** @brief  High or low GPIO state */
typedef enum {
  GPIO_STATE_LOW = 0, /**< GPIO output voltage is 0V */
  GPIO_STATE_HIGH,    /**< GPIO output voltage is 3.3V */
  NUM_GPIO_STATES,    /**< Number of GPIO states */
} GpioState;

/** @brief  Available modes for the GPIO pins */
typedef enum {
  GPIO_ANALOG = 0,        /**< Analog input pin */
  GPIO_INPUT_FLOATING,    /**< Floating input pin */
  GPIO_INPUT_PULL_DOWN,   /**< Pulled-down input pin */
  GPIO_INPUT_PULL_UP,     /**< Pulled-up input pin */
  GPIO_OUTPUT_OPEN_DRAIN, /**< Open drain output pin */
  GPIO_OUTPUT_PUSH_PULL,  /**< Push pull output pin */
  GPIO_ALTFN_OPEN_DRAIN,  /**< Alternate function Open drain pin */
  GPIO_ALTFN_PUSH_PULL,   /**< Alternate functino Push pull pin */
  NUM_GPIO_MODES,         /**< Number of GPIO Modes */
} GpioMode;

/** @brief  Alternate function mapping. Read STM32L433 datasheet page 75 */
typedef enum {
  // No ALT function
  GPIO_ALT_NONE = 0x00U, /**< No alternate function */

  // GPIO_ALT0 - System
  GPIO_ALT0_SWDIO = 0x00U, /**< Serial wire debug I/O */
  GPIO_ALT0_SWCLK = 0x00U, /**< Serial wire clock */

  // GPIO_ALT1 - TIM1/TIM2
  GPIO_ALT1_TIM1 = 0x01U, /**< Timer 1 */
  GPIO_ALT1_TIM2 = 0x01U, /**< Timer 2 */

  // GPIO_ALT4 - I2C
  GPIO_ALT4_I2C1 = 0x04U, /**< I2C Hardware block 1 */
  GPIO_ALT4_I2C2 = 0x04U, /**< I2C Hardware block 2 */
  GPIO_ALT4_I2C3 = 0x04U, /**< I2C Hardware block 3 */

  // GPIO_ALT5 - SPI
  GPIO_ALT5_SPI1 = 0x05U, /**< SPI Hardware block 1 */
  GPIO_ALT5_SPI2 = 0x05U, /**< SPI Hardware block 2 */

  // GPIO_ALT6 - SPI3
  GPIO_ALT6_SPI3 = 0x06U, /**< SPI Hardware block 3 */

  // GPIO_ALT7 - USART
  GPIO_ALT7_USART1 = 0x07U, /**< UART Hardware block 1 */
  GPIO_ALT7_USART2 = 0x07U, /**< UART Hardware block 2 */
  GPIO_ALT7_USART3 = 0x07U, /**< UART Hardware block 3 */

  // GPIO_ALT9 - CAN1
  GPIO_ALT9_CAN1 = 0x09U, /**< CAN Hardware block 1 */

  // GPIO_ALT14 - Timers
  GPIO_ALT14_TIM15 = 0x0EU, /**< Timer 15 */
  GPIO_ALT14_TIM16 = 0x0EU, /**< Timer 16 */
} GpioAlternateFunctions;

/**
 * @brief   Available GPIO ports
 */
typedef enum {
  GPIO_PORT_A = 0, /**< GPIO Port A */
  GPIO_PORT_B,     /**< GPIO Port B */
  GPIO_PORT_C,     /**< GPIO Port C */
  GPIO_PORT_D,     /**< GPIO Port D */
  GPIO_PORT_E,     /**< GPIO Port E */
  GPIO_PORT_H,     /**< GPIO Port H */
  NUM_GPIO_PORTS,  /**< Number of GPIO Ports */
} GpioPort;

#define GPIO_TOTAL_PINS ((GPIO_PINS_PER_PORT) * (NUM_GPIO_PORTS))

/**
 * @brief   Port and pin data
 */
typedef struct GpioAddress {
  GpioPort port; /**< GPIO Port can be A, B, C, D, E, or H */
  uint8_t pin;   /**< GPIO Pin must be between 0-15 */
} GpioAddress;

/**
 * @brief   Initializes GPIO globally by disabling JTAG and enabling all GPIO clocks
 * @details ONLY CALL ONCE or it will deinit all current settings. Change pin setting by calling
 *          gpio_init_pin
 * @return  STATUS_CODE_OK if intialization succeeded
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
 * @param   alt_func Alternate function of the GPIO
 * @return  STATUS_CODE_OK if pin initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode gpio_init_pin_af(const GpioAddress *address, const GpioMode pin_mode, GpioAlternateFunctions alt_func);

/**
 * @brief   Sets the GPIO pin to a valid state
 * @param   address Pointer to the GPIO address
 * @param   state GPIO state can either be HIGH/LOW
 * @return  STATUS_CODE_OK if pin writing succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode gpio_set_state(const GpioAddress *address, GpioState state);

/**
 * @brief   Toggles the GPIO
 * @param   address Pointer to the GPIO address
 * @return  STATUS_CODE_OK if pin toggling succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode gpio_toggle_state(const GpioAddress *address);

/**
 * @brief   Gets the GPIO state
 * @param   address Pointer to the GPIO address
 * @return  STATUS_CODE_OK if pin reading succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
GpioState gpio_get_state(const GpioAddress *address);

#ifdef MS_PLATFORM_X86

/**
 * @brief   Peeks at the GPIO mode
 * @param   address Pointer to the GPIO address
 * @return  GpioMode of the selected address
 */
GpioMode gpio_peek_mode(GpioAddress *address);

/**
 * @brief   Peeks at the GPIO alternate function
 * @param   address Pointer to the GPIO address
 * @return  GpioAlternateFunctions of the selected address
 */
GpioAlternateFunctions gpio_peek_alt_function(GpioAddress *address);

#endif

/** @} */
