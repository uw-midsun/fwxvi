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

static Payload makePayload(SPI::Port port, const uint8_t* data, size_t length) {
    Payload payload{};
    payload.spiPort = port;

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

std::string createSPICommand(CommandCode commandCode, std::string &SPIport, std::string data){
    try{
        switch (CommandCode) {
            case CommandCode::SPI_SET_TX_DATA: {
                m_SPIDatagram.setSPIPort(SPIport);

            }

            case CommandCode::SPI_SET_RX_DATA: {


            }
        }

        return m_SPIDatagram.serialize(commandCode);

    }

}
