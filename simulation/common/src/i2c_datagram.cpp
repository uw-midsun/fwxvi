#include "i2c_datagram.h"

#include <cstring>
#include <sstream>
#include <vector>

#include "serialization.h"

namespace Datagram {

I2C::I2C(Payload &data) {
  m_i2cDatagram = data;
}

std::string I2C::serialize(const CommandCode &commandCode) const {
  std::string serializedData;

  serializeInteger<uint8_t>(serializedData, static_cast<uint8_t>(m_i2cDatagram.i2cPort));
  serializeInteger(serializedData, static_cast<uint16_t>(m_i2cDatagram.bufferLength));
  serializedData.append(reinterpret_cast<const char *>(m_i2cDatagram.buffer), m_i2cDatagram.bufferLength);
}

void I2C::deserialize(std::string &i2cDatagramPayload) {
  size_t offset = 0;

  m_i2cDatagram.i2cPort = static_cast<Port>(deserializeInteger<uint8_t>(i2cDatagramPayload, offset));
  m_i2cDatagram.bufferLength = deserializeInteger<uint16_t>(i2cDatagramPayload, offset);

  if (m_i2cDatagram.bufferLength > MAX_BUFFER_SIZE) {
    throw std::runtime_error("Deserialized I2C buffer length exceeds maximum allowed size");
  }

  std::memcpy(m_i2cDatagram.buffer, i2cDatagramPayload.data() + offset, m_i2cDatagram.bufferLength);
}

void I2C::setI2CPort(const Port &i2cPort) {
  m_i2cDatagram.i2cPort = i2cPort;
}

void I2C::setBuffer(const uint8_t *data, size_t length) {
  std::memcpy(m_i2cDatagram.buffer, data, length);
  m_i2cDatagram.bufferLength = length;
}

void I2C::clearBuffer() {
  std::memset(m_i2cDatagram.buffer, 0U, MAX_BUFFER_SIZE);
}

I2C::Port I2C::getI2CPort() const {
  return m_i2cDatagram.i2cPort;
}

size_t I2C::getBufferLength() const {
  return m_i2cDatagram.bufferLength;
}

const uint8_t *I2C::getBuffer() const {
  return m_i2cDatagram.buffer;
}

}  // namespace Datagram
