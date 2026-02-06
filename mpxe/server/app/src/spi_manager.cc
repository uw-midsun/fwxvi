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
#include <vector>
#include <stdexcept>
#include <string>   
#include <sstream>  
#include <iostream>  
#include <iomanip>


/* Inter-component Headers */
#include "command_code.h"

/* Intra-component Headers */
#include "app.h"
#include "spi_manager.h"

#define SPI_KEY "spi"

const char *spiPortNames[] = {
  "SPI_PORT_1", 
  "SPI_PORT_2", 
};

void SPIManager::loadSPIInfo(std::string &projectName){
    m_spiInfo = serverJSONManager.getProjectValue<std::unordered_map<std::string, SPIManager::PortInfo>>(projectName, SPI_KEY);
}

void SPIManager::saveSPIInfo(std::string &projectName){
  serverJSONManager.setProjectValue(projectName, SPI_KEY, m_spiInfo);
  m_spiInfo.clear();
}



std::vector<uint8_t> SPIManager::parseHexData(const std::string &dataStr){
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

std::string SPIManager::stringifySpiBuffer(const uint8_t *buffer, size_t length){
    if (length == 0 || buffer == nullptr){
        return "None";
    }
    std::stringstream ss;
    for (size_t i = 0; i < length; ++i) {
        ss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(buffer[i]);
        if (i < length - 1){
            ss << ", ";
        }
    }
    return ss.str();
  }

void SPIManager::updateSpiReadBuffer(std::string &projectName, std::string &payload){
    loadSPIInfo(projectName);

    m_spiDatagram.deserialize(payload);

    const uint8_t *receivedData = m_spiDatagram.getBuffer();
    const uint16_t dataLength = m_spiDatagram.getBufferLength();

    using Port = Datagram::SPI::Port;
 
    if (m_spiDatagram.getSPIPort() >= Port::NUM_SPI_PORTS) {
        throw std::runtime_error("Error: SPI payload contains invalid port ID!");
         
    }

    std::string portKey = spiPortNames[static_cast<uint8_t>(m_spiDatagram.getSPIPort())];

    m_spiInfo[portKey]["Data"] = stringifySpiBuffer(receivedData, dataLength);

    saveSPIInfo(projectName);
}




std::string SPIManager::createSpiCommand(CommandCode commandCode, std::string &spiPort, std::string data){
    try{
        if (spiPort.length() <= 9 || spiPort.substr(0, 9) != "SPI_PORT_") {
            throw std::runtime_error("Invalid SPI Port format. Good Example: SPI_PORT_1");
        }
        
        uint8_t portNum = static_cast<uint8_t>(std::stoi(spiPort.substr(9)));
        Datagram::SPI::Port port = static_cast<Datagram::SPI::Port>(portNum - 1);

        if (port >= Datagram::SPI::Port::NUM_SPI_PORTS) {
            throw std::runtime_error("Invalid SPI port number");
        }
        
        switch (commandCode){   
            case CommandCode::SPI_READ_DATA:{
                m_spiDatagram.setSPIPort(port);
                m_spiDatagram.setBuffer(nullptr, 0U);
                break;
            }
            case CommandCode::SPI_WRITE_DATA:
            case CommandCode::SPI_TRANSFER_DATA:{
                auto bytes = parseHexData(data);
                
                if (bytes.size() > Datagram::SPI::SPI_MAX_BUFFER_SIZE) {
                    throw std::runtime_error("Data exceeds maximum SPI buffer size.");
                }
                
                m_spiDatagram.setSPIPort(port);
                m_spiDatagram.setBuffer(bytes.data(), bytes.size());
                break;

            }

            case CommandCode::SPI_CLEAR_BUFFER:{
                m_spiDatagram.setSPIPort(port);
                m_spiDatagram.clearBuffer();
                break;
            }
                
            default: {
                throw std::runtime_error("Invalid command code");
                break;
            }
        }
        return m_spiDatagram.serialize(commandCode);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return "";
    
}

