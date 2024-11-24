#pragma once

/************************************************************************************************
 * @file   gpio_mcu.h
 *
 * @brief  Header file for MCU specific GPIO library
 *
 * @date   2024-11-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */

/* Intra-component Headers */

/** 
 * @defgroup  GPIO
 * @brief     GPIO library
 * @{
 */

#ifndef GPIO_PINS_PER_PORT
#define GPIO_PINS_PER_PORT 16
#endif

#ifndef GPIO_ADDRESS_OFFSET
#define GPIO_ADDRESS_OFFSET 0x400U
#endif

/**
 * @brief   High or low GPIO state
 */
typedef enum {
  GPIO_STATE_LOW = 0,
  GPIO_STATE_HIGH,
  NUM_GPIO_STATES,
} GpioState;

/**
 * @brief   Available modes for the GPIO pins
 */
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
 * @brief   Alternate function mapping. Read STM32L433 datasheet page 75
 */
typedef enum {
  // No ALT function
  GPIO_ALT_NONE = 0x00U,

  // GPIO_ALT0 - System
  GPIO_ALT0_SWDIO = 0x00U,
  GPIO_ALT0_SWCLK = 0x00U,

  // GPIO_ALT1 - TIM1/TIM2
  GPIO_ALT1_TIM1 = 0x01U,
  GPIO_ALT1_TIM2 = 0x01U,

  // GPIO_ALT4 - I2C
  GPIO_ALT4_I2C1 = 0x04U,
  GPIO_ALT4_I2C2 = 0x04U,
  GPIO_ALT4_I2C3 = 0x04U,

  // GPIO_ALT5 - SPI
  GPIO_ALT5_SPI1 = 0x05U,
  GPIO_ALT5_SPI2 = 0x05U,

  // GPIO_ALT6 - SPI3
  GPIO_ALT6_SPI3 = 0x06U,

  // GPIO_ALT7 - USART
  GPIO_ALT7_USART1 = 0x07U,
  GPIO_ALT7_USART2 = 0x07U,
  GPIO_ALT7_USART3 = 0x07U,

  // GPIO_ALT9 - CAN1
  GPIO_ALT9_CAN1 = 0x09U,

  // GPIO_ALT14 - Timers
  GPIO_ALT14_TIM15 = 0x0EU,
  GPIO_ALT14_TIM16 = 0x0EU,
} GpioAlternateFunctions;

/**
 * @brief   Available GPIO ports
 */
typedef enum {
  GPIO_PORT_A = 0,
  GPIO_PORT_B,
  GPIO_PORT_C,
  GPIO_PORT_D,
  GPIO_PORT_E,
  GPIO_PORT_H,
  NUM_GPIO_PORTS,
} GpioPort;

#define GPIO_TOTAL_PINS ((GPIO_PINS_PER_PORT) * (NUM_GPIO_PORTS))

/** @} */
