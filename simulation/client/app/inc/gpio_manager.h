#ifndef GPIO_MANAGER_H
#define GPIO_MANAGER_H

#include <stdint.h>
#include <string.h>

#include <unordered_map>

#include "gpio_datagram.h"

class GpioManager {
 private:
  Datagram::Gpio m_gpioDatagram;

 public:
  GpioManager() = default;

  void setGpioPinState(std::string &payload);
  void setGpioAllStates(std::string &payload);

  std::string processGpioPinState(std::string &payload);
  std::string processGpioAllStates();

  std::string processGpioPinMode(std::string &payload);
  std::string processGpioAllModes();

  std::string processGpioPinAltFunction(std::string &payload);
  std::string processGpioAllAltFunctions();
};

#endif
