/************************************************************************************************
 * @file   spi_manager.cc
 *
 * @brief  Source file defining the SPIManager Class for the server
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>

/* Inter-component Headers */
#include "command_code.h"

/* Intra-component Headers */
#include "app.h"
#include "spi_manager.h"

#define SPI_KEY "spi"

    if (length > SPI_MAX_BUFFER_SIZE) {
        throw std::runtime_error("Payload size exceeds SPI_MAX_BUFFER_SIZE");
    }

    std::memcpy(payload.buffer, data, length);
    payload.bufferLength = length;

    return payload;
}

void SPIManager::loadSPIInfo(std::string &projectName){
    m_SPIInfo = serverJSONManager.getProjectValue<std::unordered_map<std::string, SPIManager::SPIObjectInfo>>(projectName, SPI_KEY);
}

void SPIManager::saveSPIInfo(std::string &projectName){
  serverJSONManager.setProjectValue(projectName, SPI_KEY, m_SPIInfo);
  m_SPIInfo.clear();
}

// Converts "0x12, 0x14, 0x56, 0x17" → {0x12, 0x14, 0x56, 0x17}
std::vector<uint8_t> parseHexData(const std::string &dataStr) {
    std::vector<uint8_t> bytes;
    std::stringstream ss(dataStr);
    std::string token;

    while (std::getline(ss, token, ',')) {
        // Trim whitespace
        token.erase(0, token.find_first_not_of(" \t\n\r"));
        token.erase(token.find_last_not_of(" \t\n\r") + 1);

        if (token.rfind("0x", 0) != 0 && token.rfind("0X", 0) != 0) {
            throw std::invalid_argument("Invalid hex format: " + token);
        }

        uint16_t value;
        std::stringstream hexstream(token);
        hexstream >> std::hex >> value;

        if (value > 0xFF) {
            throw std::out_of_range("Value exceeds 1 byte: " + token);
        }

        bytes.push_back(static_cast<uint8_t>(value));
    }

    return bytes;
}

std::string SPIManager::createSPICommand(CommandCode commandCode, const std::string &SPIport, const std::string &data) {
    Datagram::SPI m_SPIDatagram;

    // Convert port string → enum
    Datagram::SPI::Port port;
    if (SPIport == "SPI_PORT_1") {
        port = Datagram::SPI::Port::SPI_PORT_1;
    } else if (SPIport == "SPI_PORT_2") {
        port = Datagram::SPI::Port::SPI_PORT_2;
    } else {
        throw std::invalid_argument("Invalid SPI port string: " + SPIport);
    }

    switch (commandCode) {
        case CommandCode::SPI_SET_TX_DATA: {
            m_SPIDatagram.setSPIPort(port);

            auto bytes = parseHexData(data);
            m_SPIDatagram.setBuffer(bytes.data(), bytes.size());
            break;
        }

        case CommandCode::SPI_SET_RX_DATA: {
            m_SPIDatagram.setSPIPort(port);
            // RX might just request data from client → no buffer needed
            break;
        }

        default:
            throw std::invalid_argument("Unsupported SPI CommandCode");
    }

    return m_SPIDatagram.serialize(commandCode);
}
