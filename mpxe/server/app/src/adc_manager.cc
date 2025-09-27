

/* Standard library Headers */
#include <cstdint>

/* Inter-component Headers */
#include "command_code.h"

/* Intra-component Headers */
#include "app.h"
#include "adc_manager.h"

#define RAW_READINGS "raw_readings"
#define CONV_READINGS "converted_readings"
#define ADC_KEY "adc"


std::string stringifyReadings(const uint8_t *buffer, uint8_t bufferLength) {
    std::string result;
    std::size_t count = bufferLength / sizeof(uint16_t);
    for (size_t i = 0; i < count; i++) {
        uint16_t value = static_cast<uint16_t>(buffer[2 * i]) |
                         (static_cast<uint16_t>(buffer[2 * i + 1]) << 8);

        result += std::to_string(value);   
        if (i + 1 < count) {
            result += ", ";
        }
    }

    return result;
}

std::string AdcManager::stringifyGpioAddress(GpioPort port, uint8_t pin) {
    for (const Mapping &entry : channelMap) {
      //TODO Why do I get VSCode error? I might be dyslexic
        if (entry.port == port && entry.pin == pin) {
            char portLetter = "ABC"[static_cast<int>(port)];
            return "Channel " + std::to_string(entry.channel) +
                   " -> " + portLetter + std::to_string(pin);
        }
    }
    return "Invalid mapping";
}

void AdcManager::loadAdcInfo(std::string &projectName) {
  m_adcInfo = serverJSONManager.getProjectValue<std::unordered_map<std::string, AdcReadingInfo>>(projectName, ADC_KEY); 
}

void AdcManager::saveAdcInfo(std::string &projectName) {
  serverJSONManager.setProjectValue(projectName, ADC_KEY, m_adcInfo); 
  
  m_adcInfo.clear();
}

void AdcManager::updateAdcRaw(std::string &projectName, std::string &payload) {
  loadAdcInfo(projectName);
  m_adcDatagram.deserialize(payload);

  const uint8_t *receivedData = m_adcDatagram.getBuffer(); 
  const uint8_t dataLength = m_adcDatagram.getBufferLength(); 
  
  std::string dataString = stringifyReadings(receivedData, dataLength);

  GpioPort port = m_adcDatagram.getGpioPort(); 
  uint8_t pin = m_adcDatagram.getGpioPin(); 

  std::string GpioAddress = stringifyGpioAddress(port, pin); 
  
  m_adcInfo[RAW_READINGS][GpioAddress] = dataString; 
  saveAdcInfo(projectName);
}

void AdcManager::updateAdcRawAll(std::string &projectName, std::string &payload) {
  loadAdcInfo(projectName);
  m_adcDatagram.deserialize(payload);

  const uint8_t *receivedData = m_adcDatagram.getBuffer(); 
  const uint8_t dataLength = m_adcDatagram.getBufferLength(); 

  std::string dataString = stringifyReadings(receivedData, dataLength);
  
  for (const Mapping &entry : channelMap) {
    std::string gpioAddress = stringifyGpioAddress(entry.port, entry.pin);
    m_adcInfo[RAW_READINGS][gpioAddress] = dataString;
  } 

  saveAdcInfo(projectName);
} 

void AdcManager::updateAdcConverted(std::string &projectName, std::string &payload) {
  loadAdcInfo(projectName);
  m_adcDatagram.deserialize(payload);
  
  const uint8_t *receivedData = m_adcDatagram.getBuffer(); 
  const uint8_t dataLength = m_adcDatagram.getBufferLength(); 
  
  std::string dataString = stringifyReadings(receivedData, dataLength);

  GpioPort port = m_adcDatagram.getGpioPort(); 
  uint8_t pin = m_adcDatagram.getGpioPin(); 

  std::string GpioAddress = stringifyGpioAddress(port, pin); 
  
  m_adcInfo[CONV_READINGS][GpioAddress] = dataString; 
  saveAdcInfo(projectName);
}

void AdcManager::updateAdcConvertedAll(std::string &projectName, std::string &payload) {
  loadAdcInfo(projectName);
  m_adcDatagram.deserialize(payload);
  m_adcDatagram.deserialize(payload);

  const uint8_t *receivedData = m_adcDatagram.getBuffer(); 
  const uint8_t dataLength = m_adcDatagram.getBufferLength(); 

  std::string dataString = stringifyReadings(receivedData, dataLength);
  
  for (const Mapping &entry : channelMap) {
    std::string gpioAddress = stringifyGpioAddress(entry.port, entry.pin);
    m_adcInfo[CONV_READINGS][gpioAddress] = dataString;
  } 
  saveAdcInfo(projectName);
} 

//TODO Add error checks for GPIO Address
std::string AdcManager::createAdcCommand(CommandCode commandCode, std::string gpioAddress, std::string reading) {
  try {
    switch (commandCode) {
      case CommandCode::ADC_SET_RAW: { 
        GpioPort port = static_cast<GpioPort>(gpioAddress[0] - 'A'); 
        uint8_t pin = static_cast<uint8_t>(std::stoi(gpioAddress.substr(1))); 
        uint16_t readingValue = static_cast<uint16_t>(std::stoi(reading)); 

        m_adcDatagram.setGpioPin(pin);
        m_adcDatagram.setGpioPort(port); 
        m_adcDatagram.setBuffer(reinterpret_cast<const uint8_t *>(&readingValue), 2);

        break;
      }
      case CommandCode::ADC_SET_ALL_RAW: {
        uint16_t readingValue = static_cast<uint16_t>(std::stoi(reading)); 
        m_adcDatagram.setBuffer(reinterpret_cast<const uint8_t *>(&readingValue), 2); 
        break;
      }
      //TODO Not sure if being used, might just remove it
      // case CommandCode::ADC_SET_CONVERTED: {
      //   GpioPort port = static_cast<GpioPort>(gpioAddress[0] - 'A'); 
      //   uint8_t pin = static_cast<uint8_t>(std::stoi(gpioAddress.substr(1))); 

      //   m_adcDatagram.setGpioPin(pin);
      //   m_adcDatagram.setGpioPort(port); 
      //   break;
      // }
      // case CommandCode::ADC_SET_ALL_CONVERTED: {
      //   break;
      // }
      case CommandCode::ADC_GET_RAW: {
        GpioPort port = static_cast<GpioPort>(gpioAddress[0] - 'A'); 
        uint8_t pin = static_cast<uint8_t>(std::stoi(gpioAddress.substr(1))); 

        m_adcDatagram.setGpioPin(pin);
        m_adcDatagram.setGpioPort(port); 
        break;
      }
      case CommandCode::ADC_GET_ALL_RAW: {
        break;
      }
      case CommandCode::ADC_GET_CONVERTED: {
        GpioPort port = static_cast<GpioPort>(gpioAddress[0] - 'A'); 
        uint8_t pin = static_cast<uint8_t>(std::stoi(gpioAddress.substr(1))); 

        m_adcDatagram.setGpioPin(pin);
        m_adcDatagram.setGpioPort(port); 
        break;
      }
      case CommandCode::ADC_GET_ALL_CONVERTED: {
        break;
      }
      default: {
        throw std::runtime_error("Invalid command code");
        break;
      }
    }
    return m_adcDatagram.serialize(commandCode); 
  }
  catch (std::exception &e) {
    std::cerr << "Adc Manager error: " << e.what() << std::endl;
  }
  return "";
} 