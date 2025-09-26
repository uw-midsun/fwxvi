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

/* Inter-component Headers */
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

  using SPIObjectInfo = std::unordered_map<std::string, std::string>;
  std::unordered_map<std::string, SPIObjectInfo> m_SPIInfo;
  Datagram::SPI m_SPIDatagram;

  void loadSPIInfo(std::string &projectName);

  void saveSPIInfo(std::string &projectName);

 public:
  /**
   * @brief   Constructs a SPIManager object
   * @details Default constructor
   */

  SPIManager() = default;


  std::string createSPICommand(CommandCode commandCode, std::string &SPIport, std::string data);

};

/** @} */
