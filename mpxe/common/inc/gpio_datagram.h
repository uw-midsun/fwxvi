#pragma once

/************************************************************************************************
 * @file   gpio_datagram.h
 *
 * @brief  Header file defining the GpioDatagram class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <string>

/* Inter-component Headers */

/* Intra-component Headers */
#include "command_code.h"

/**
 * @defgroup GpioDatagram
 * @brief    Shared Gpio Datagram class
 * @{
 */

namespace Datagram {

/**
 * @class   DatagramGpio
 * @brief   Class for managing Gpio port configurations and data transfer operations
 * @details This class provides an interface for configuring and managing Gpio ports, including
 *          pin modes, states, and alternate functions. It supports serialization for data transfer
 *          and maintains a buffer for Gpio operations
 */
class Gpio {
 public:
  static const constexpr uint8_t PINS_PER_PORT = 16U; /**< Supported Pins per Port */

  /**
   * @brief   Gpio Port definition
   */
  enum class Port {
    GPIO_PORT_A = 0, /**< Gpio port A */
    GPIO_PORT_B,     /**< Gpio port B */
    NUM_GPIO_PORTS   /**< Number of Gpio Ports */
  };

  /**
   * @brief   Gpio State definition
   */
  enum class State {
    GPIO_STATE_LOW = 0, /**< Gpio state low */
    GPIO_STATE_HIGH,    /**< Gpio state high */
  };

  /**
   * @brief   Gpio Mode definition
   */
  enum class Mode {
    GPIO_ANALOG = 0,        /**< Gpio Mode Analog */
    GPIO_INPUT_FLOATING,    /**< Gpio Mode Input Floating */
    GPIO_INPUT_PULL_DOWN,   /**< Gpio Mode Input Pull Down */
    GPIO_INPUT_PULL_UP,     /**< Gpio Mode Input Pull Up */
    GPIO_OUTPUT_OPEN_DRAIN, /**< Gpio Mode Output Open Drain */
    GPIO_OUTPUT_PUSH_PULL,  /**< Gpio Mode Output Push Pull */
    GPIO_ALFTN_OPEN_DRAIN,  /**< Gpio Mode Alt Open Drain */
    GPIO_ALTFN_PUSH_PULL,   /**< Gpio Mode Alt Push Pull */
    NUM_GPIO_MODES,         /**< Number of Gpio Modes */
  };

  /**
   * @brief   Gpio Alt function definition
   */
  enum class AltFunction {
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
  };

  /** Maximum buffer size dependent on the Pins per port and Number of Gpio ports */
  static constexpr size_t GPIO_MAX_BUFFER_SIZE = PINS_PER_PORT * static_cast<uint8_t>(Port::NUM_GPIO_PORTS);

  /**
   * @brief   Gpio Datagram payload storage
   */
  struct Payload {
    Port gpioPort;                        /**< Gpio port */
    uint8_t gpioPin;                      /**< Gpio pin */
    uint8_t bufferLength;                 /**< Data buffer length */
    uint8_t buffer[GPIO_MAX_BUFFER_SIZE]; /**< Data buffer */
  };

  /**
   * @brief   Constructs a Gpio object with provided payload data
   * @param   data Reference to payload data
   */
  explicit Gpio(Payload &data);

  /**
   * @brief   Default constructor for Gpio object
   */
  Gpio() = default;

  /**
   * @brief   Serializes Gpio data with command code for transmission
   * @param   commandCode Command code to include in serialized data
   * @return  Serialized string containing Gpio data
   */
  std::string serialize(const CommandCode &commandCode) const;

  /**
   * @brief   Deserializes Gpio data from payload string
   * @param   gpioDatagramPayload String containing serialized Gpio data
   */
  void deserialize(std::string &gpioDatagramPayload);

  /**
   * @brief   Sets the target Gpio port
   * @param   gpioPort Port to set as target
   */
  void setGpioPort(const Port &gpioPort);

  /**
   * @brief   Sets the target Gpio pin
   * @param   gpioPin Pin to set as target
   */
  void setGpioPin(const uint8_t &gpioPin);

  /**
   * @brief   Sets data in the Gpio buffer
   * @param   data Pointer to data to copy into buffer
   * @param   length Length of data to copy
   */
  void setBuffer(const uint8_t *data, uint8_t length);

  /**
   * @brief   Clear the Gpio data buffer
   */
  void clearBuffer();

  /**
   * @brief   Gets the target Gpio port
   * @return  Target port
   */
  Port getGpioPort() const;

  /**
   * @brief   Gets the target Gpio pin
   * @return  Target pin
   */
  uint8_t getGpioPin() const;

  /**
   * @brief   Gets the Gpio buffer length
   * @return  Buffer length
   */
  uint8_t getBufferLength() const;

  /**
   * @brief   Gets the Gpio buffer
   * @return  Pointer to the buffer array
   */
  const uint8_t *getBuffer() const;

 private:
  Payload m_gpioDatagram; /**< Private datagram payload */
};

}  // namespace Datagram

/** @} */
