#pragma once

/************************************************************************************************
 * @file   spi_manager.h
 *
 * @brief  Header file defining the Client SPIManager class
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
 * @defgroup ClientSPIManager
 * @brief    SPIManager for the Client
 * @{
 */

/**
 * @class   ClientSPIManager
 * @brief   Class that manages receiving and transmitting SPI commands and JSON logging
 * @details This class is responsible for handling and transmitting serialized messages for setting/retrieving RX/TX data buffers
 *          The class shall provide an interface to control SPI transactions at the driver-level
 */
class SPIManager {
 private:
  Datagram::SPI m_spiDatagram; /**<Datagram class to serialize/deserialize commands */

 public:
  /**
   * @brief   Constructs a SPIManager object
   * @details Default constructor
   */
  SPIManager() = default;

  /**
   * @brief   Transmits data to SPI RX buffer
   * @param   payload Serialized SPI datagram payload containing port and data
   * @return  Serialized acknowledgement response
   */
  std::string writeSpiData(std::string &payload);

  /**
   * @brief   Reads data from the SPI TX buffer written from spi_write()
   * @param   payload Serialized SPI datagram payload containing port and data
   * @return  Serialized response with TX buffer data
   */
  std::string processReadSpiData(std::string &payload);

  /**
   * @brief   Transmits
   * @details This function shall be called upon receiving a pin-specific payload
   * @param   payload Message data payload to be parsed
   */
  std::string transferSpiData(std::string &payload);

  /**
   * @brief   Clears both SPI RX and TX buffers
   * @param   payload Serialized SPI datagram payload containing port and data
   */
  void clearBuffer(std::string &payload);
};

/** @} */
