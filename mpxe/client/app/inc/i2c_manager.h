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
  Datagram::I2C m_I2CDatagram; /**< Datagram class to serialize/deserialize commands */
 public:
  /**
   * @brief   Constructs a I2CManager object
   * @details Default constructor
   */
  I2CManager() = default;

  /**
   * @brief sets
   * @details etet
   *  
   * @param payload message data 
   */
  std::string I2CManager::I2C_get_data(std::string &payload);

  /**
   * @brief erre
   * @details ere
   * 
   * @param payload message d
   */
  std::string I2CManager::I2C_set_data(std::string &payload);
};

/** @} */
