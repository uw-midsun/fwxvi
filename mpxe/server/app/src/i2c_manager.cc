
/************************************************************************************************
 * @file   i2c_manager.cc
 *
 * @brief  Source file defining the I2CManager Class for the server
 *
 * @date   2026-02-05
 * @author Jun Li
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>

/* Inter-component Headers */
#include "command_code.h"

/* Intra-component Headers */
#include "app.h"
#include "i2c_manager.h"

#define I2C_KEY "i2c"
#define DATA_KEY "data"

const char *i2cPortNames[] = {
  "I2C1", /* I2C_PORT_1 */
  "I2C2", /* I2C_PORT_2 */
};

std::string I2CManager::stringifyData(const std::vector<uint8_t> &data) {
  std::stringstream ss;
  
  ss << "[";
  for (size_t i = 0; i < data.size(); i++) {
    ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << static_cast<int>(data[i]);
    if (i < data.size() - 1) {
      ss << ", ";
    }
  }
  ss << "]";
  
  return ss.str();
}

void I2CManager::loadI2CInfo(std::string &projectName) {
  m_i2cInfo = serverJSONManager.getProjectValue<std::unordered_map<std::string, I2CManager::PortInfo>>(projectName, I2C_KEY);
}

void I2CManager::saveI2CInfo(std::string &projectName) {
  serverJSONManager.setProjectValue(projectName, I2C_KEY, m_i2cInfo);

  /* Upon save, clear the memory */
  m_i2cInfo.clear();
}

void I2CManager::updateI2CData(std::string &projectName, std::string &payload) {
  loadI2CInfo(projectName);

  m_I2CDatagram.deserialize(payload);

  std::string key = i2cPortNames[static_cast<uint8_t>(m_I2CDatagram.getI2CPort())];
  const uint8_t *receivedData = m_I2CDatagram.getBuffer();
  size_t length = m_I2CDatagram.getBufferLength();

  /* Store data as vector and stringify it for JSON */
  std::vector<uint8_t> dataVector(receivedData, receivedData + length);
  m_i2cInfo[key][DATA_KEY] = stringifyData(dataVector);

  saveI2CInfo(projectName);
}

std::string I2CManager::createI2CCommand(CommandCode commandCode, std::string &i2cPort, std::vector<uint8_t> &data) {
  try {
    switch (commandCode) {
      case CommandCode::I2C_SET_DATA: {
        if (i2cPort.empty() || i2cPort.size() < 4) {
          throw std::runtime_error(
              "Invalid format for I2C port specification. Good examples: 'I2C1' "
              "'I2C2'");
          break;
        }

        /* Extract port number from string (e.g., "I2C1" -> 1) */
        uint8_t portNum = static_cast<uint8_t>(std::stoi(i2cPort.substr(3))) - 1;

        if (portNum >= static_cast<uint8_t>(Datagram::I2C::Port::NUM_I2C_PORTS)) {
          throw std::runtime_error("Invalid selection for I2C ports. Expected: I2C1 or I2C2");
          break;
        }

        if (data.size() > Datagram::I2C::I2C_MAX_BUFFER_SIZE) {
          throw std::runtime_error("Data size exceeds maximum I2C buffer size of " + 
                                   std::to_string(Datagram::I2C::I2C_MAX_BUFFER_SIZE));
          break;
        }

        m_I2CDatagram.setI2CPort(static_cast<Datagram::I2C::Port>(portNum));
        m_I2CDatagram.setBuffer(data.data(), data.size());

        break;
      }

      case CommandCode::I2C_GET_DATA: {
        if (i2cPort.empty() || i2cPort.size() < 4) {
          throw std::runtime_error(
              "Invalid format for I2C port specification. Good examples: 'I2C1' "
              "'I2C2'");
          break;
        }

        /* Extract port number from string (e.g., "I2C1" -> 1) */
        uint8_t portNum = static_cast<uint8_t>(std::stoi(i2cPort.substr(3))) - 1;

        if (portNum >= static_cast<uint8_t>(Datagram::I2C::Port::NUM_I2C_PORTS)) {
          throw std::runtime_error("Invalid selection for I2C ports. Expected: I2C1 or I2C2");
          break;
        }

        m_I2CDatagram.setI2CPort(static_cast<Datagram::I2C::Port>(portNum));
        m_I2CDatagram.setBuffer(nullptr, 0U);

        break;
      }

      default: {
        throw std::runtime_error("Invalid command code");
        break;
      }
    }

    return m_I2CDatagram.serialize(commandCode);
  } catch (std::exception &e) {
    std::cerr << "I2C Manager error: " << e.what() << std::endl;
  }
  return "";
}