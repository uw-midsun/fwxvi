

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


std::string AdcManager::stringifyReadings(const uint8_t *&buffer, size_t bufferLength) {
  if (bufferLength % 2 != 0) {
    throw std::runtime_error("ADC payload length not even");
  }

  std::string result;
  bool first = true;

  for (size_t remaining = bufferLength; remaining >= 2; remaining -= 2) {
    uint16_t value = static_cast<uint16_t>(buffer[0]) | (static_cast<uint16_t>(buffer[1]) << 8);
    buffer += 2;

    if (!first) 
      result += ", ";

    first = false;

    result += std::to_string(value);
  }

  return result;
}

std::string AdcManager::stringifyGpioAddress(GpioPort port, uint8_t pin) {
  char portLetter = "ABC"[static_cast<int>(port)];
  return std::string(1, portLetter) + std::to_string(pin);
}

std::string AdcManager::getChannel(GpioPort port, uint8_t pin) {
  for (const Mapping &entry : channelMap) {
    if (entry.port == port && entry.pin == pin) {
      std::string channelNumString = (entry.channel < 10) ? "0" + std::to_string(entry.channel) : std::to_string(entry.channel);
      return "channel " + channelNumString;
    }
  }
  return "Invalid Channel"; 
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
  
  std::string channelKey = getChannel(port, pin);

  m_adcInfo[RAW_READINGS][channelKey]["Gpio Port"] = stringifyGpioAddress(port, pin); 
  m_adcInfo[RAW_READINGS][channelKey]["Reading"] = dataString;

  saveAdcInfo(projectName);
}

void AdcManager::updateAdcRawAll(std::string &projectName, std::string &payload) {
  loadAdcInfo(projectName);
  m_adcDatagram.deserialize(payload);

  const uint8_t *receivedData = m_adcDatagram.getBuffer(); 
  const uint8_t dataLength = m_adcDatagram.getBufferLength(); 
  
  for (const Mapping &entry : channelMap) {
    std::string channelKey = getChannel(entry.port, entry.pin);

    m_adcInfo[RAW_READINGS][channelKey]["Gpio Port"] = stringifyGpioAddress(entry.port, entry.pin); 
    m_adcInfo[RAW_READINGS][channelKey]["Reading"] = stringifyReadings(receivedData, 2);
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

  std::string channelKey = getChannel(port, pin);
  m_adcInfo[CONV_READINGS][channelKey]["Gpio Port"] = stringifyGpioAddress(port, pin); 
  m_adcInfo[CONV_READINGS][channelKey]["Reading"] = dataString;

  saveAdcInfo(projectName);
}

void AdcManager::updateAdcConvertedAll(std::string &projectName, std::string &payload) {
  loadAdcInfo(projectName);
  m_adcDatagram.deserialize(payload);

  const uint8_t *receivedData = m_adcDatagram.getBuffer(); 
  const uint8_t dataLength = m_adcDatagram.getBufferLength(); 
  
  for (const Mapping &entry : channelMap) {
    std::string channelKey = getChannel(entry.port, entry.pin);
    m_adcInfo[CONV_READINGS][channelKey]["Gpio Port"] = stringifyGpioAddress(entry.port, entry.pin); 
    m_adcInfo[CONV_READINGS][channelKey]["Reading"] = stringifyReadings(receivedData, 2);
  } 
  saveAdcInfo(projectName);
} 

std::string AdcManager::createAdcCommand(CommandCode commandCode, std::string gpioAddress, std::string reading) {
  try {
    switch (commandCode) {
      case CommandCode::ADC_SET_RAW: { 
        if (gpioAddress.empty() || gpioAddress.size() < 2) {
          throw std::runtime_error(
              "Invalid format for port/pin specification. Good examples: 'A0' "
              "'C1' 'B1'");
          break;
        }
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
      case CommandCode::ADC_GET_RAW: 
      case CommandCode::ADC_GET_CONVERTED: {
        if (gpioAddress.empty() || gpioAddress.size() < 2) {
          throw std::runtime_error(
              "Invalid format for port/pin specification. Good examples: 'A0' "
              "'C1' 'B1'");
          break;
        }
        GpioPort port = static_cast<GpioPort>(gpioAddress[0] - 'A'); 
        uint8_t pin = static_cast<uint8_t>(std::stoi(gpioAddress.substr(1))); 

        m_adcDatagram.setGpioPin(pin);
        m_adcDatagram.setGpioPort(port); 
        break;
      }
      case CommandCode::ADC_GET_ALL_RAW: 
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
