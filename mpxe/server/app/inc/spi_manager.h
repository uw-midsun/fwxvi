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
  /** @brief Hash map to store SPI port and buffer data */
  using PortInfo = std::unordered_map<std::string, std::string>;

  std::unordered_map<std::string, PortInfo> m_spiInfo; /**< Hash-map to cache all SPI data */
  Datagram::SPI m_spiDatagram;                         /**< Datagram object to serialize/deserialize payloads */

  /**
   * @brief   Load cached SPI data for a project from JSON storage
   * @param   projectName Name of the project whose SPI info should be loaded
   */
  void loadSPIInfo(std::string &projectName);

  /**
   * @brief   Save cached SPI data for a project to JSON storage
   * @param   projectName Name of the project whose SPI info should be saved
   */
  void saveSPIInfo(std::string &projectName);

  /**
   * @brief   Parse a hex string into a byte vector
   * @param   dataStr Input string containing hex values
   * @return  std::vector<uint8_t> Parsed byte vector
   */
  std::vector<uint8_t> parseHexData(const std::string &dataStr);

  /**
   * @brief   Convert SPI buffer contents to a string
   * @param   buffer Pointer to the data buffer
   * @param   length Number of bytes in the buffer
   * @return  std::string String representation of the buffer
   */
  std::string stringifySpiBuffer(const uint8_t *buffer, size_t length);

 public:
  /**
   * @brief   Construct a new SPI Manager object
   * @details Default constructor
   */
  SPIManager() = default;

  /**
   * @brief   Update SPI read buffer for a specific SPI port
   * @param   projectName Project name for which data is updated
   * @param   payload Serialized data payload containing the buffer
   */
  void updateSpiReadBuffer(std::string &projectName, std::string &payload);

  /**
   * @brief   Create a serialized SPI command message
   * @param   commandCode SPI command type to execute
   * @param   spiPort Target SPI port identifier
   * @param   data Hex string data to transmit
   * @return  std::string Serialized command string
   */
  std::string createSpiCommand(CommandCode commandCode, std::string &spiPort, std::string data);
};

/** @} */
