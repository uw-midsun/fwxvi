/************************************************************************************************
 * @file   gpio_datagram.cc
 *
 * @brief  Source file defining the AdcDatagram class
 *
 * @date   2025-03-28
 * @author Shahzaib Siddiqui
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

/* Inter-component Headers */

/* Intra-component Headers */
#include "adc_datagram.h"
#include "serialization.h"

namespace Datagram {

Adc::Adc(Payload &data) {
  m_adcDatagram = data;
}

std::string Adc::serialize(const CommandCode &commandCode) const {
  std::string serializedData;

  serializeInteger<uint8_t>(serializedData, static_cast<uint8_t>(m_adcDatagram.gpioPort));
  serializeInteger<uint8_t>(serializedData, static_cast<uint8_t>(m_adcDatagram.gpioPin));

  if (m_adcDatagram.bufferLength > ADC_MAX_BUFFER_SIZE) {
    throw std::runtime_error("Serialized ADC buffer length exceeds maximum allowed size");
  }

  serializeInteger<uint8_t>(serializedData, m_adcDatagram.bufferLength);
  serializedData.append(reinterpret_cast<const char *>(m_adcDatagram.buffer), m_adcDatagram.bufferLength);
  return encodeCommand(commandCode, serializedData);
}

void Adc::deserialize(std::string &AdcDatagramPayload) {
  if (AdcDatagramPayload.size() < 3U) {
    throw std::runtime_error("Invalid ADC datagram payload");
  }
  size_t offset = 0;

  m_adcDatagram.gpioPort = static_cast<Port>(deserializeInteger<uint8_t>(AdcDatagramPayload, offset));
  m_adcDatagram.gpioPin = deserializeInteger<uint8_t>(AdcDatagramPayload, offset);
  m_adcDatagram.bufferLength = deserializeInteger<uint8_t>(AdcDatagramPayload, offset);

  if (m_adcDatagram.bufferLength > ADC_MAX_BUFFER_SIZE) {
    throw std::runtime_error("Deserialized Adc buffer length exceeds maximum allowed size");
  }

  std::memcpy(m_adcDatagram.buffer, AdcDatagramPayload.data() + offset, m_adcDatagram.bufferLength);
}

void Adc::setGpioPort(const Port &gpioPort) {
  m_adcDatagram.gpioPort = gpioPort;
}

void Adc::setGpioPin(const uint8_t &gpioPin) {
  m_adcDatagram.gpioPin = gpioPin;
}

void Adc::setBuffer(const uint8_t *data, uint8_t length) {
  std::memcpy(m_adcDatagram.buffer, data, length);
  m_adcDatagram.bufferLength = length;
}

void Adc::clearBuffer() {
  std::memset(m_adcDatagram.buffer, 0U, ADC_MAX_BUFFER_SIZE);
}

Adc::Port Adc::getGpioPort() const {
  return m_adcDatagram.gpioPort;
}

uint8_t Adc::getGpioPin() const {
  return m_adcDatagram.gpioPin;
}

uint8_t Adc::getBufferLength() const {
  return m_adcDatagram.bufferLength;
}

const uint8_t *Adc::getBuffer() const {
  return m_adcDatagram.buffer;
}

}  // namespace Datagram
