#pragma once
/* Standard library Headers */
#include <stdint.h>
#include <string.h>

#include <unordered_map>

/* Inter-component Headers */
#include "adc_datagram.h"

/* Intra-component Headers */

class AdcManager {
 private: 
  using AdcReadingInfo = std::unordered_map<std::string, std::string>; 
  std::unordered_map<std::string, AdcReadingInfo> m_adcInfo; 
  
  using GpioPort = Datagram::Adc::Port; 
  Datagram::Adc m_adcDatagram; 

  struct Mapping {
    GpioPort port; 
    uint8_t pin; 
    uint8_t channel; 
  };

  static constexpr Mapping channelMap[] = {
    {GpioPort::GPIO_PORT_C, 0,  1}, {GpioPort::GPIO_PORT_C, 1,  2}, {GpioPort::GPIO_PORT_C, 2,  3},
    {GpioPort::GPIO_PORT_C, 3,  4}, {GpioPort::GPIO_PORT_A, 0,  5}, {GpioPort::GPIO_PORT_A, 1,  6}, 
    {GpioPort::GPIO_PORT_A, 2,  7}, {GpioPort::GPIO_PORT_A, 3,  8}, {GpioPort::GPIO_PORT_A, 4,  9},
    {GpioPort::GPIO_PORT_A, 5, 10}, {GpioPort::GPIO_PORT_A, 6, 11}, {GpioPort::GPIO_PORT_A, 7, 12},
    {GpioPort::GPIO_PORT_C, 4, 13}, {GpioPort::GPIO_PORT_C, 5, 14}, {GpioPort::GPIO_PORT_B, 0, 15},
    {GpioPort::GPIO_PORT_B, 1, 16}
  };

  void loadAdcInfo(std::string &projectName); 
  void saveAdcInfo(std::string &projectName); 
  std::string stringifyGpioAddress(GpioPort port, uint8_t pin); 

 public: 
  AdcManager() = default; 

  void updateAdcRaw(std::string &projectName, std::string &payload);
  
  void updateAdcRawAll(std::string &projectName, std::string &payload); 

  void updateAdcConverted(std::string &projectName, std::string &payload);

  void updateAdcConvertedAll(std::string &projectName, std::string &payload); 

  std::string createAdcCommand(CommandCode commandCode, std::string gpioAddress, std::string readings); 
};
