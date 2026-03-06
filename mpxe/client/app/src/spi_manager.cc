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
#include "gpio.h"
#include "spi.h"
}

#include "command_code.h"

/* Intra-component Headers */
#include "app.h"
#include "gpio_manager.h"
#include "spi_manager.h"

void SPIManager::writeSpiData(std::string &payload) {
  m_spiDatagram.deserialize(payload);

  // Get Spi Port and Data
  SpiPort port = static_cast<SpiPort>(m_spiDatagram.getSPIPort());
  const u_int8_t *receivedData = m_spiDatagram.getBuffer();
  u_int8_t s = *receivedData;
  size_t len = m_spiDatagram.getBufferLength();

  // Write New Data
  u_int8_t data = spi_tx(port, &s, len);
  m_spiDatagram.setBuffer(&data, sizeof(data));
  // m_spiDatagram.clearBuffer();
}

std::string SPIManager::processReadSpiData(std::string &payload) {
  m_spiDatagram.deserialize(payload);

  // Get Spi Port and Data
  SpiPort port = static_cast<SpiPort>(m_spiDatagram.getSPIPort());
  size_t len = m_spiDatagram.getBufferLength();
  const u_int8_t *receivedData = m_spiDatagram.getBuffer();
  u_int8_t s = *receivedData;
  u_int8_t data = static_cast<u_int8_t>(spi_rx(port, &s, len));

  // m_spiDatagram.clearBuffer();
  m_spiDatagram.setBuffer(&data, len);
  return m_spiDatagram.serialize(CommandCode::SPI_READ_DATA);
}

void transferSpiData(std::string &payload) {}

void SPIManager::clearBuffer(std::string &payload) {
  // Clear Data in Port Address
  const Datagram::SPI::Port port = m_spiDatagram.getSPIPort();
  m_spiDatagram.setSPIPort(port);
  m_spiDatagram.clearBuffer();
}