#pragma once

/************************************************************************************************
 * @file   i2c_datagram.h
 *
 * @brief  Header file defining the I2CDatagram class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <string>

/* Inter-component Headers */

/* Intra-component Headers */
#include "command_code.h"

/**
 * @defgroup I2CDatagram
 * @brief    Shared I2C Datagram class
 * @{
 */

namespace Datagram {

/**
 * @class   DatagramI2C
 * @brief   Class for managing I2C port configurations and data transfer operations
 * @details This class provides an interface for configuring and managing I2C TX/RX data
 *          on selected ports. It supports serialization for data transfer and maintains
 *          a buffer for I2C operations
 */
class I2C {
 public:
  static constexpr size_t I2C_MAX_BUFFER_SIZE = 256; /**< Maximum permitted buffer size */

  /**
   * @brief   I2C Port definition
   */
  enum class Port {
    I2C_PORT_1,   /**< I2C Port 1 */
    I2C_PORT_2,   /**< I2C Port 2 */
    NUM_I2C_PORTS /**< Number of I2C ports */
  };

  /**
   * @brief   I2C Datagram payload storage
   */
  struct Payload {
    Port i2cPort;                        /**< I2C port */
    size_t bufferLength;                 /**< Data buffer length */
    uint8_t buffer[I2C_MAX_BUFFER_SIZE]; /**< Data buffer */
  };

  /**
   * @brief   Constructs an I2C object with provided payload data
   * @param   data Reference to payload data
   */
  explicit I2C(Payload &data);

  /**
   * @brief   Default constructor for I2C object
   */
  I2C() = default;

  /**
   * @brief   Serializes I2C data with command code for transmission
   * @param   commandCode Command code to include in serialized data
   * @return  Serialized string containing I2C data
   */
  std::string serialize(const CommandCode &commandCode) const;

  /**
   * @brief   Deserializes I2C data from payload string
   * @param   i2cDatagramPayload String containing serialized I2C data
   */
  void deserialize(std::string &i2cDatagramPayload);

  /**
   * @brief   Sets the target I2C port
   * @param   i2cPort Port to set as target
   */
  void setI2CPort(const Port &i2cPort);

  /**
   * @brief   Sets data in the I2C buffer
   * @param   data Pointer to data to copy into buffer
   * @param   length Length of data to copy
   */
  void setBuffer(const uint8_t *data, size_t length);

  /**
   * @brief   Clear the I2C data buffer
   */
  void clearBuffer();

  /**
   * @brief   Gets the target I2C port
   * @return  Target port
   */
  Port getI2CPort() const;

  /**
   * @brief   Gets the I2C buffer length
   * @return  Buffer length
   */
  size_t getBufferLength() const;

  /**
   * @brief   Gets the I2C buffer
   * @return  Pointer to the buffer array
   */
  const uint8_t *getBuffer() const;

 private:
  Payload m_i2cDatagram; /**< Private datagram payload */
};

}  // namespace Datagram

/** @} */
