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

  /*
  SPI COMMANDS JUST FOR ME TO KEEP FOR NOW
  SPI_WRITE_DATA,
  SPI_READ_DATA,
  SPI_TRANSFER_DATA,
  SPI_CLEAR_BUFFER,
  */

  /**
   * @brief   Sets the raw ADC value given the data payload
   * @details This function shall be called upon receiving a pin-specific payload
   * @param   payload Message data payload to be parsed
   */
  void writeSpiData(std::string &payload);

  /**
   * @brief   Sets the raw ADC value given the data payload
   * @details This function shall be called upon receiving a pin-specific payload
   * @param   payload Message data payload to be parsed
   */
  std::string processReadSpiData(std::string &payload);

  /**
   * @brief   Sets the raw ADC value given the data payload
   * @details This function shall be called upon receiving a pin-specific payload
   * @param   payload Message data payload to be parsed
   */
  void transferSpiData();

  /**
   * @brief   Clears the SPI buffer given the data payload
   * @details This function shall be called upon receiving a port-specific payload
   * @param   payload Message data payload to be parsed
   */
  void clearBuffer(std::string &payload);
};

/** @} */
