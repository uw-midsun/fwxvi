#pragma once

/************************************************************************************************
 * @file   spi_datagram.h
 *
 * @brief  Header file defining the SPIDatagram class
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
 * @defgroup SPIDatagram
 * @brief    Shared SPI Datagram class
 * @{
 */

namespace Datagram {

/**
 * @class   DatagramSPI
 * @brief   Class for managing SPI port configurations and data transfer operations
 * @details This class provides an interface for configuring and managing SPI TX/RX data
 *          on selected ports. It supports serialization for data transfer and maintains
 *          a buffer for SPI operations
 */
class SPI {
 public:
  static constexpr size_t SPI_MAX_BUFFER_SIZE = 256; /**< Maximum permitted buffer size */

  /**
   * @brief   SPI Port definition
   */
  enum class Port {
    SPI_PORT_1,   /**< SPI Port 1 */
    SPI_PORT_2,   /**< SPI Port 2 */
    NUM_SPI_PORTS /**< Number of SPI ports */
  };

  /**
   * @brief   SPI Datagram payload storage
   */
  struct Payload {
    Port spiPort;                        /**< SPI port */
    size_t bufferLength;                 /**< Data buffer length */
    uint8_t buffer[SPI_MAX_BUFFER_SIZE]; /**< Data buffer */
  };

  /**
   * @brief   Constructs a SPI object with provided payload data
   * @param   data Reference to payload data
   */
  explicit SPI(Payload &data);

  /**
   * @brief   Default constructor for SPI object
   */
  SPI() = default;

  /**
   * @brief   Serializes SPI data with command code for transmission
   * @param   commandCode Command code to include in serialized data
   * @return  Serialized string containing SPI data
   */
  std::string serialize(const CommandCode &commandCode) const;

  /**
   * @brief   Deserializes SPI data from payload string
   * @param   spiDatagramPayload String containing serialized SPI data
   */
  void deserialize(std::string &spiDatagramPayload);

  /**
   * @brief   Sets the target SPI port
   * @param   spiPort Port to set as target
   */
  void setSPIPort(const Port &spiPort);

  /**
   * @brief   Sets data in the SPI buffer
   * @param   data Pointer to data to copy into buffer
   * @param   length Length of data to copy
   */
  void setBuffer(const uint8_t *data, size_t length);

  /**
   * @brief   Clear the SPI data buffer
   */
  void clearBuffer();

  /**
   * @brief   Gets the target SPI port
   * @return  Target port
   */
  Port getSPIPort() const;

  /**
   * @brief   Gets the SPI buffer length
   * @return  Buffer length
   */
  size_t getBufferLength() const;

  /**
   * @brief   Gets the SPI buffer
   * @return  Pointer to the buffer array
   */
  const uint8_t *getBuffer() const;

 private:
  Payload m_spiDatagram; /**< Private datagram payload */
};

}  // namespace Datagram

/** @} */
