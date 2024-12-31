#include "gpio_manager.h"

#include <cstdint>

#include "app.h"
#include "command_code.h"

#define GPIO_KEY "gpio"
#define PIN_STATE_KEY "state"
#define PIN_MODE_KEY "mode"
#define PIN_ALT_FUNC_KEY "alternate_function"

const char *gpioPortNames[] = {
    "A", /* GPIO_PORT_A */
    "B", /* GPIO_PORT_B */
};

std::string GpioManager::stringifyPinMode(Datagram::Gpio::Mode mode) {
  std::string result = "";

  switch (mode) {
    case Datagram::Gpio::Mode::GPIO_ANALOG: {
      result = "Analog";
      break;
    }
    case Datagram::Gpio::Mode::GPIO_INPUT_FLOATING: {
      result = "Floating Input";
      break;
    }
    case Datagram::Gpio::Mode::GPIO_INPUT_PULL_DOWN: {
      result = "Pull-down Input";
      break;
    }
    case Datagram::Gpio::Mode::GPIO_INPUT_PULL_UP: {
      result = "Pull-up Input";
      break;
    }
    case Datagram::Gpio::Mode::GPIO_OUTPUT_OPEN_DRAIN: {
      result = "Open-drain Output";
      break;
    }
    case Datagram::Gpio::Mode::GPIO_OUTPUT_PUSH_PULL: {
      result = "Push-pull Output";
      break;
    }
    case Datagram::Gpio::Mode::GPIO_ALFTN_OPEN_DRAIN: {
      result = "Open-drain Alternate Function";
      break;
    }
    case Datagram::Gpio::Mode::GPIO_ALTFN_PUSH_PULL: {
      result = "Push-pull Alternate Function";
      break;
    }
    default: {
      result = "Invalid Mode";
      break;
    }
  }

  return result;
}

std::string GpioManager::stringifyPinAltFunction(Datagram::Gpio::AltFunction altFunction) {
  std::string result = "";

  switch (altFunction) {
    /* Duplicate value is SWCLK/SWDIO */
    case Datagram::Gpio::AltFunction::GPIO_ALT_NONE: {
      result = "None/SWDIO/SWCLK ALT0";
      break;
    }
    /* Duplicate value is Timer 2 */
    case Datagram::Gpio::AltFunction::GPIO_ALT1_TIM1: {
      result = "Timer1/2 ALT1";
      break;
    }
    /* Duplicate values is I2C2/3 */
    case Datagram::Gpio::AltFunction::GPIO_ALT4_I2C1: {
      result = "I2C1/2/3 ALT4";
      break;
    }
    /* Duplicate value is SPI2 */
    case Datagram::Gpio::AltFunction::GPIO_ALT5_SPI1: {
      result = "SPI1/2 ALT5";
      break;
    }
    case Datagram::Gpio::AltFunction::GPIO_ALT6_SPI3: {
      result = "SPI3 ALT6";
      break;
    }
    case Datagram::Gpio::AltFunction::GPIO_ALT7_USART1: {
      result = "USART1/2/3 ALT7";
      break;
    }
    case Datagram::Gpio::AltFunction::GPIO_ALT9_CAN1: {
      result = "CAN1 ALT9";
      break;
    }
    /* Duplicate value is Timer 16 */
    case Datagram::Gpio::AltFunction::GPIO_ALT14_TIM15: {
      result = "TIMER15/16 ALT14";
      break;
    }
    default: {
      result = "Invalid Alternate Function";
    }
  }

  return result;
}

void GpioManager::loadGpioInfo(std::string &projectName) {
  m_gpioInfo = globalJSON.getProjectValue<std::unordered_map<std::string, GpioManager::PinInfo>>(projectName, GPIO_KEY);
}

void GpioManager::saveGpioInfo(std::string &projectName) {
  globalJSON.setProjectValue(projectName, GPIO_KEY, m_gpioInfo);

  /* Upon save, clear the memory */
  m_gpioInfo.clear();
}

void GpioManager::updateGpioPinState(std::string &projectName, std::string &payload) {
  loadGpioInfo(projectName);

  m_gpioDatagram.deserialize(payload);

  std::string key = gpioPortNames[static_cast<uint8_t>(m_gpioDatagram.getGpioPort())];
  key += std::to_string(m_gpioDatagram.getGpioPin());
  const uint8_t *receivedData = m_gpioDatagram.getBuffer();

  if (static_cast<Datagram::Gpio::State>(receivedData[0U]) == Datagram::Gpio::State::GPIO_STATE_HIGH) {
    m_gpioInfo[key][PIN_STATE_KEY] = "HIGH";
  } else if (static_cast<Datagram::Gpio::State>(receivedData[0U]) == Datagram::Gpio::State::GPIO_STATE_LOW) {
    m_gpioInfo[key][PIN_STATE_KEY] = "LOW";
  } else {
    m_gpioInfo[key][PIN_STATE_KEY] = "INVALID";
  }

  saveGpioInfo(projectName);
}

void GpioManager::updateGpioAllStates(std::string &projectName, std::string &payload) {
  loadGpioInfo(projectName);

  m_gpioDatagram.deserialize(payload);

  const uint8_t *receivedData = m_gpioDatagram.getBuffer();

  for (uint8_t i = 0U; i < static_cast<uint8_t>(Datagram::Gpio::Port::NUM_GPIO_PORTS) * static_cast<uint8_t>(Datagram::Gpio::PINS_PER_PORT); i++) {
    size_t blockIndex = i / 8U;
    size_t bitPosition = i % 8U;
    bool pinState = (receivedData[blockIndex] & (1U << bitPosition)) != 0;
    std::string key = gpioPortNames[i / Datagram::Gpio::PINS_PER_PORT];
    key += std::to_string(i % Datagram::Gpio::PINS_PER_PORT);

    if (pinState) {
      m_gpioInfo[key][PIN_STATE_KEY] = "HIGH";
    } else {
      m_gpioInfo[key][PIN_STATE_KEY] = "LOW";
    }
  }

  saveGpioInfo(projectName);
}

void GpioManager::updateGpioPinMode(std::string &projectName, std::string &payload) {
  loadGpioInfo(projectName);

  m_gpioDatagram.deserialize(payload);

  std::string key = gpioPortNames[static_cast<uint8_t>(m_gpioDatagram.getGpioPort())];
  key += std::to_string(m_gpioDatagram.getGpioPin());

  const uint8_t *receivedData = m_gpioDatagram.getBuffer();

  m_gpioInfo[key][PIN_MODE_KEY] = stringifyPinMode(static_cast<Datagram::Gpio::Mode>(receivedData[0U]));

  saveGpioInfo(projectName);
}

void GpioManager::updateGpioAllModes(std::string &projectName, std::string &payload) {
  loadGpioInfo(projectName);

  m_gpioDatagram.deserialize(payload);

  const uint32_t *receivedData = reinterpret_cast<const uint32_t *>(m_gpioDatagram.getBuffer());

  for (uint8_t i = 0U; i < static_cast<uint8_t>(Datagram::Gpio::Port::NUM_GPIO_PORTS) * static_cast<uint8_t>(Datagram::Gpio::PINS_PER_PORT); i++) {
    size_t blockIndex = (i / 8U);     /* 4 bits per pin so there is only 8 pins per block */
    size_t bitOffset = (i % 8U) * 4U; /* Multiply by 4 because each each mode is 4 bit */

    uint8_t pinMode = (receivedData[blockIndex] >> bitOffset) & 0x0F;

    std::string key = gpioPortNames[i / Datagram::Gpio::PINS_PER_PORT];
    key += std::to_string(i % Datagram::Gpio::PINS_PER_PORT);

    m_gpioInfo[key][PIN_MODE_KEY] = stringifyPinMode(static_cast<Datagram::Gpio::Mode>(pinMode));
  }

  saveGpioInfo(projectName);
}

void GpioManager::updateGpioPinAltFunction(std::string &projectName, std::string &payload) {
  loadGpioInfo(projectName);

  m_gpioDatagram.deserialize(payload);

  std::string key = gpioPortNames[static_cast<uint8_t>(m_gpioDatagram.getGpioPort())];
  key += std::to_string(m_gpioDatagram.getGpioPin());

  const uint8_t *receivedData = m_gpioDatagram.getBuffer();

  m_gpioInfo[key][PIN_ALT_FUNC_KEY] = stringifyPinAltFunction(static_cast<Datagram::Gpio::AltFunction>(receivedData[0U]));

  saveGpioInfo(projectName);
}

void GpioManager::updateGpioAllAltFunctions(std::string &projectName, std::string &payload) {
  loadGpioInfo(projectName);

  m_gpioDatagram.deserialize(payload);

  const uint32_t *receivedData = reinterpret_cast<const uint32_t *>(m_gpioDatagram.getBuffer());

  for (uint8_t i = 0U; i < static_cast<uint8_t>(Datagram::Gpio::Port::NUM_GPIO_PORTS) * static_cast<uint8_t>(Datagram::Gpio::PINS_PER_PORT); i++) {
    size_t blockIndex = (i / 8U);     /* 4 bits per pin so there is only 8 pins per block */
    size_t bitOffset = (i % 8U) * 4U; /* Multiply by 4 because each each mode is 4 bit */

    uint8_t pinAltFunction = (receivedData[blockIndex] >> bitOffset) & 0x0F;

    std::string key = gpioPortNames[i / Datagram::Gpio::PINS_PER_PORT];
    key += std::to_string(i % Datagram::Gpio::PINS_PER_PORT);

    m_gpioInfo[key][PIN_ALT_FUNC_KEY] = stringifyPinAltFunction(static_cast<Datagram::Gpio::AltFunction>(pinAltFunction));
  }

  saveGpioInfo(projectName);
}

std::string GpioManager::createGpioCommand(CommandCode commandCode, std::string &gpioPortPin, std::string data) {
  try {
    switch (commandCode) {
      case CommandCode::GPIO_GET_PIN_STATE:
      case CommandCode::GPIO_GET_PIN_MODE:
      case CommandCode::GPIO_GET_PIN_ALT_FUNCTION: {
        if (gpioPortPin.empty() || gpioPortPin.size() < 2) {
          throw std::runtime_error(
              "Invalid format for port/pin specification. Good examples: 'A9' "
              "'A12' 'B13'");
          break;
        }

        Datagram::Gpio::Port port = static_cast<Datagram::Gpio::Port>(gpioPortPin[0] - 'A');
        uint8_t pin = static_cast<uint8_t>(std::stoi(gpioPortPin.substr(1)));

        if (port >= Datagram::Gpio::Port::NUM_GPIO_PORTS) {
          throw std::runtime_error("Invalid selection for Gpio ports. Expected: A or B");
          break;
        }

        if (pin >= Datagram::Gpio::PINS_PER_PORT) {
          throw std::runtime_error("Exceeded maximum number of Gpio pins: " + std::to_string(static_cast<int>(Datagram::Gpio::PINS_PER_PORT)));
          break;
        }

        m_gpioDatagram.setGpioPort(port);
        m_gpioDatagram.setGpioPin(pin);
        m_gpioDatagram.setBuffer(nullptr, 0U);
        break;
      }

      case CommandCode::GPIO_GET_ALL_STATES:
      case CommandCode::GPIO_GET_ALL_MODES:
      case CommandCode::GPIO_GET_ALL_ALT_FUNCTIONS: {
        m_gpioDatagram.setGpioPort(Datagram::Gpio::Port::NUM_GPIO_PORTS);
        m_gpioDatagram.setGpioPin(Datagram::Gpio::PINS_PER_PORT);
        m_gpioDatagram.setBuffer(nullptr, 0U);
        break;
      }

      case CommandCode::GPIO_SET_PIN_STATE: {
        if (gpioPortPin.empty() || gpioPortPin.size() < 2) {
          throw std::runtime_error(
              "Invalid format for port/pin specification. Good examples: 'A9' "
              "'A12' 'B13'");
          break;
        }

        Datagram::Gpio::Port port = static_cast<Datagram::Gpio::Port>(gpioPortPin[0] - 'A');
        uint8_t pin = static_cast<uint8_t>(std::stoi(gpioPortPin.substr(1)));
        uint8_t pinState;

        if (port >= Datagram::Gpio::Port::NUM_GPIO_PORTS) {
          throw std::runtime_error("Invalid selection for Gpio ports. Expected: A or B");
          break;
        }

        if (pin >= Datagram::Gpio::PINS_PER_PORT) {
          throw std::runtime_error("Exceeded maximum number of Gpio pins: " + std::to_string(static_cast<int>(Datagram::Gpio::PINS_PER_PORT)));
          break;
        }

        if (data == "HIGH") {
          pinState = static_cast<uint8_t>(Datagram::Gpio::State::GPIO_STATE_HIGH);
        } else if (data == "LOW") {
          pinState = static_cast<uint8_t>(Datagram::Gpio::State::GPIO_STATE_LOW);
        } else {
          throw std::runtime_error("Invalid Gpio state: " + data);
          break;
        }

        m_gpioDatagram.setGpioPort(port);
        m_gpioDatagram.setGpioPin(pin);
        m_gpioDatagram.setBuffer(&pinState, sizeof(pinState));

        break;
      }

      case CommandCode::GPIO_SET_ALL_STATES: {
        uint8_t pinState;
        if (data == "HIGH") {
          pinState = static_cast<uint8_t>(Datagram::Gpio::State::GPIO_STATE_HIGH);
        } else if (data == "LOW") {
          pinState = static_cast<uint8_t>(Datagram::Gpio::State::GPIO_STATE_LOW);
        } else {
          throw std::runtime_error("Invalid Gpio state: " + data);
          break;
        }

        m_gpioDatagram.setGpioPort(Datagram::Gpio::Port::NUM_GPIO_PORTS);
        m_gpioDatagram.setGpioPin(Datagram::Gpio::PINS_PER_PORT);
        m_gpioDatagram.setBuffer(&pinState, sizeof(pinState));

        break;
      }

      default: {
        throw std::runtime_error("Invalid command code");
        break;
      }
    }

    return m_gpioDatagram.serialize(commandCode);

  } catch (std::exception &e) {
    std::cerr << "Gpio Manager error: " << e.what() << std::endl;
  }
  return "";
}
