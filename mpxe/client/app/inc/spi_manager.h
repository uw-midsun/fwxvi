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
 public:
  /**
   * @brief   Constructs a SPIManager object
   * @details Default constructor
   */
  SPIManager() = default;
};

/** @} */
