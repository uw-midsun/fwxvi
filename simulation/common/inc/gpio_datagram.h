#ifndef GPIO_DATAGRAM_H
#define GPIO_DATAGRAM_H

#include <cstdint>
#include <string>

#include "command_code.h"

namespace Datagram {
class Gpio {
 public:
  static const constexpr uint8_t PINS_PER_PORT = 16U;

  enum class Port { GPIO_PORT_A = 0, GPIO_PORT_B, NUM_GPIO_PORTS };

  enum class State {
    GPIO_STATE_LOW = 0,
    GPIO_STATE_HIGH,
  };

  enum class Mode {
    GPIO_ANALOG = 0,
    GPIO_INPUT_FLOATING,
    GPIO_INPUT_PULL_DOWN,
    GPIO_INPUT_PULL_UP,
    GPIO_OUTPUT_OPEN_DRAIN,
    GPIO_OUTPUT_PUSH_PULL,
    GPIO_ALFTN_OPEN_DRAIN,
    GPIO_ALTFN_PUSH_PULL,
    NUM_GPIO_MODES,
  };

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

  static constexpr size_t GPIO_MAX_BUFFER_SIZE = PINS_PER_PORT * static_cast<uint8_t>(Port::NUM_GPIO_PORTS);

  struct Payload {
    Port gpioPort;
    uint8_t gpioPin;
    uint8_t bufferLength;
    uint8_t buffer[GPIO_MAX_BUFFER_SIZE];
  };

  explicit Gpio(Payload &data);
  Gpio() = default;

  std::string serialize(const CommandCode &commandCode) const;
  void deserialize(std::string &gpioDatagramPayload);

  void setGpioPort(const Port &gpioPort);
  void setGpioPin(const uint8_t &gpioPin);
  void setBuffer(const uint8_t *data, uint8_t length);
  void clearBuffer();

  Port getGpioPort() const;
  uint8_t getGpioPin() const;
  uint8_t getBufferLength() const;
  const uint8_t *getBuffer() const;

 private:
  Payload m_gpioDatagram;
};

}  // namespace Datagram
#endif
