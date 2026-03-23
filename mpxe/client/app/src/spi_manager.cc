/************************************************************************************************
 * @file   spi_manager.cc
 *
 * @brief  Source file defining the SPIManager class for the client
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <iostream>
#include <vector>

/* Inter-component Headers */
extern "C" {
#include "spi.h"
}

#include "command_code.h"

/* Intra-component Headers */
#include "app.h"
#include "spi_manager.h"

std::string SPIManager::writeSpiData(std::string &payload) {
  m_spiDatagram.deserialize(payload);

  // Get Spi Port and Data
  SpiPort port = static_cast<SpiPort>(m_spiDatagram.getSPIPort());
  const u_int8_t *receivedData = m_spiDatagram.getBuffer();
  size_t len = m_spiDatagram.getBufferLength();

  spi_set_rx(port, receivedData, static_cast<uint8_t>(len));
  std::cout << "BRKPT1 - received buffer length: " << len << std::endl;

  m_spiDatagram.clearBuffer();

  return m_spiDatagram.serialize(CommandCode::SPI_WRITE_DATA);
}

std::string SPIManager::processReadSpiData(std::string &payload) {
  m_spiDatagram.deserialize(payload);

  // Get Spi Port and Data
  SpiPort port = static_cast<SpiPort>(m_spiDatagram.getSPIPort());
  size_t len = m_spiDatagram.getBufferLength();

  if (len == 0U) {
    std::cout << "DATAGRAM BUFFER_LEN IS " << len << std::endl;
    len = spi_get_tx_num_bytes(port);
    std::cout << "PORT IS " << port << std::endl;
  }

  u_int8_t tmp[len];
  spi_get_tx_data(port, tmp, static_cast<u_int8_t>(len));

  std::cout << "DRIVER BUFFER_LEN IS " << len << std::endl;

  m_spiDatagram.setBuffer(tmp, len);
  return m_spiDatagram.serialize(CommandCode::SPI_READ_DATA);
}

std::string SPIManager::transferSpiData(std::string &payload) {
  m_spiDatagram.deserialize(payload);

  // Get Spi Port and Data
  SpiPort port = static_cast<SpiPort>(m_spiDatagram.getSPIPort());
  size_t len = m_spiDatagram.getBufferLength(); 

  spi_exchange(port, )
}

void SPIManager::clearBuffer(std::string &payload) {
  // Clear Data in Port Address
  const Datagram::SPI::Port port = m_spiDatagram.getSPIPort();
  m_spiDatagram.setSPIPort(port);
  m_spiDatagram.clearBuffer();
}