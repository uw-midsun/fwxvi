#pragma once

/************************************************************************************************
 * @file   spi_manager.h
 *
 * @brief  Header file defining the Server SPIManager class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>
#include <string.h>

#include <unordered_map>
#include <vector>

/* Inter-component Headers */
#include "command_code.h"
#include "spi_datagram.h"

/* Intra-component Headers */

/**
 * @defgroup ServerSPIManager
 * @brief    SPIManager for the Server
 * @{
 */

/**
 * @class   ServerSPIManager
 * @brief   Class that manages receiving and transmitting SPI commands and JSON logging
 * @details This class is responsible for transmitting serialized messages for setting/retrieving RX/TX data buffers
 *          The class shall provide an interface to control SPI transactions
 */
class SPIManager {
 private:
  using PortInfo = std::unordered_map<std::string, std::string>;

  std::unordered_map<std::string, PortInfo> m_spiInfo;
  Datagram::SPI m_spiDatagram;

  void loadSPIInfo(std::string &projectName);

  void saveSPIInfo(std::string &projectName);

  std::vector<uint8_t> parseHexData(const std::string &dataStr);

  std::string stringifySpiBuffer(const uint8_t *buffer, size_t length);

 public:
  /**
   * @brief   Constructs a SPIManager object
   * @details Default constructor
   */
  SPIManager() = default;

  void updateSpiReadBuffer(std::string &projectName, std::string &payload);

  std::string createSpiCommand(CommandCode commandCode, std::string &spiPort, std::string data);
};

/** @} */
