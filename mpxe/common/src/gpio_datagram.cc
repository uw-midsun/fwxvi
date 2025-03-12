/************************************************************************************************
 * @file   gpio_datagram.cc
 *
 * @brief  Source file defining the GpioDatagram class
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
#include "gpio_datagram.h"
#include "serialization.h"

namespace Datagram {
std::string Gpio::serialize(const CommandCode &commandCode) const {
  std::string serializedData;

  serializeInteger<uint8_t>(serializedData, static_cast<uint8_t>(m_gpioDatagram.gpioPort));
  serializeInteger<uint8_t>(serializedData, static_cast<uint8_t>(m_gpioDatagram.gpioPin));

  if (m_gpioDatagram.bufferLength > GPIO_MAX_BUFFER_SIZE) {
    throw std::runtime_error("Serialized Gpio buffer length exceeds maximum allowed size");
  }

  serializeInteger<uint8_t>(serializedData, m_gpioDatagram.bufferLength);
  serializedData.append(reinterpret_cast<const char *>(m_gpioDatagram.buffer), m_gpioDatagram.bufferLength);
  return encodeCommand(commandCode, serializedData);
}

void Gpio::deserialize(std::string &gpioDatagramPayload) {
  if (gpioDatagramPayload.size() < 3U) {
    throw std::runtime_error("Invalid GPIO datagram payload");
  }
  size_t offset = 0;

  m_gpioDatagram.gpioPort = static_cast<Port>(deserializeInteger<uint8_t>(gpioDatagramPayload, offset));
  m_gpioDatagram.gpioPin = deserializeInteger<uint8_t>(gpioDatagramPayload, offset);
  m_gpioDatagram.bufferLength = deserializeInteger<uint8_t>(gpioDatagramPayload, offset);

  if (m_gpioDatagram.bufferLength > GPIO_MAX_BUFFER_SIZE) {
    throw std::runtime_error("Deserialized Gpio buffer length exceeds maximum allowed size");
  }

  std::memcpy(m_gpioDatagram.buffer, gpioDatagramPayload.data() + offset, m_gpioDatagram.bufferLength);
}

Gpio::Gpio(Payload &data) {
  m_gpioDatagram = data;
}

void Gpio::setGpioPort(const Port &gpioPort) {
  m_gpioDatagram.gpioPort = gpioPort;
}

void Gpio::setGpioPin(const uint8_t &gpioPin) {
  m_gpioDatagram.gpioPin = gpioPin;
}

void Gpio::setBuffer(const uint8_t *data, uint8_t length) {
  std::memcpy(m_gpioDatagram.buffer, data, length);
  m_gpioDatagram.bufferLength = length;
}

void Gpio::clearBuffer() {
  std::memset(m_gpioDatagram.buffer, 0U, GPIO_MAX_BUFFER_SIZE);
}

Gpio::Port Gpio::getGpioPort() const {
  return m_gpioDatagram.gpioPort;
}

uint8_t Gpio::getGpioPin() const {
  return m_gpioDatagram.gpioPin;
}

uint8_t Gpio::getBufferLength() const {
  return m_gpioDatagram.bufferLength;
}

const uint8_t *Gpio::getBuffer() const {
  return m_gpioDatagram.buffer;
}

}  // namespace Datagram
