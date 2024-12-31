#ifndef GPIO_MANAGER_H
#define GPIO_MANAGER_H

#include <stdint.h>
#include <string.h>

#include <unordered_map>

#include "gpio_datagram.h"

class GpioManager {
 private:
  using PinInfo = std::unordered_map<std::string, std::string>;

  std::unordered_map<std::string, PinInfo> m_gpioInfo;

  Datagram::Gpio m_gpioDatagram;

  std::string stringifyPinMode(Datagram::Gpio::Mode mode);
  std::string stringifyPinAltFunction(Datagram::Gpio::AltFunction altFunction);

  void loadGpioInfo(std::string &projectName);
  void saveGpioInfo(std::string &projectName);

 public:
  GpioManager() = default;

  void updateGpioPinState(std::string &projectName, std::string &payload);
  void updateGpioAllStates(std::string &projectName, std::string &payload);

  void updateGpioPinMode(std::string &projectName, std::string &payload);
  void updateGpioAllModes(std::string &projectName, std::string &payload);

  void updateGpioPinAltFunction(std::string &projectName, std::string &payload);
  void updateGpioAllAltFunctions(std::string &projectName, std::string &payload);

  std::string createGpioCommand(CommandCode commandCode, std::string &gpioPortPin, std::string data);
};

#endif
