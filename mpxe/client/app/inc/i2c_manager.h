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
   * @brief   Writes data into the I2C RX buffer so firmware i2c_read() can consume it
   * @param   payload Serialized I2C datagram payload containing port and data
   * @return  Serialized acknowledgement response
   */
  std::string writeI2CData(std::string &payload);

  /**
   * @brief   Reads data from the I2C TX buffer that firmware wrote via i2c_write()
   * @param   payload Serialized I2C datagram payload containing port and length
   * @return  Serialized response with TX buffer data
   */
  std::string readI2CData(std::string &payload);

  /**
   * @brief   Clears both the I2C RX and TX buffers
   * @param   payload Serialized I2C datagram payload containing port
   * @return  Serialized acknowledgement response
   */
  std::string clearI2CBuffers(std::string &payload);
};

/** @} */
