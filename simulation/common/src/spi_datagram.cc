/************************************************************************************************
 * @file   spi_datagram.cc
 *
 * @brief  Source file defining the SPIDatagram class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <cstring>
#include <iostream>
#include <vector>

/* Inter-component Headers */

/* Intra-component Headers */
#include "serialization.h"
#include "spi_datagram.h"

namespace Datagram {

SPI::SPI(Payload &data) {
  m_spiDatagram = data;
}

std::string SPI::serialize(const CommandCode &commandCode) const {
  std::string serializedData;

  serializeInteger<uint8_t>(serializedData, static_cast<uint8_t>(m_spiDatagram.spiPort));
  serializeInteger(serializedData, static_cast<uint16_t>(m_spiDatagram.bufferLength));
  serializedData.append(reinterpret_cast<const char *>(m_spiDatagram.buffer), m_spiDatagram.bufferLength);
}

void SPI::deserialize(std::string &spiDatagramPayload) {
  size_t offset = 0;

  m_spiDatagram.spiPort = static_cast<Port>(deserializeInteger<uint8_t>(spiDatagramPayload, offset));
  m_spiDatagram.bufferLength = deserializeInteger<uint16_t>(spiDatagramPayload, offset);

  if (m_spiDatagram.bufferLength > SPI_MAX_BUFFER_SIZE) {
    throw std::runtime_error("Deserialized SPI buffer length exceeds maximum allowed size");
  }

  std::memcpy(m_spiDatagram.buffer, spiDatagramPayload.data() + offset, m_spiDatagram.bufferLength);
}

void SPI::setSPIPort(const Port &spiPort) {
  m_spiDatagram.spiPort = spiPort;
}

void SPI::setBuffer(const uint8_t *data, size_t length) {
  std::memcpy(m_spiDatagram.buffer, data, length);
  m_spiDatagram.bufferLength = length;
}

void SPI::clearBuffer() {
  std::memset(m_spiDatagram.buffer, 0U, SPI_MAX_BUFFER_SIZE);
}

SPI::Port SPI::getSPIPort() const {
  return m_spiDatagram.spiPort;
}

size_t SPI::getBufferLength() const {
  return m_spiDatagram.bufferLength;
}

const uint8_t *SPI::getBuffer() const {
  return m_spiDatagram.buffer;
}

}  // namespace Datagram
