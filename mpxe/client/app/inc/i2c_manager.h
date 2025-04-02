#pragma once

/************************************************************************************************
 * @file   i2c_manager.h
 *
 * @brief  Header file defining the Client I2CManager class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>
#include <string.h>

/* Inter-component Headers */
#include "i2c_datagram.h"

/* Intra-component Headers */

/**
 * @defgroup ClientI2CManager
 * @brief    I2CManager for the Client
 * @{
 */

/**
 * @class   ClientI2CManager
 * @brief   Class that manages receiving and transmitting I2C commands and JSON logging
 * @details This class is responsible for handling and transmitting serialized messages for setting/retrieving RX/TX data buffers
 *          The class shall provide an interface to control I2C transactions at the driver-level
 */
class I2CManager {
 private:
 public:
  /**
   * @brief   Constructs a I2CManager object
   * @details Default constructor
   */
  I2CManager() = default;
};

/** @} */
