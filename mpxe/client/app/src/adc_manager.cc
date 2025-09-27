
/************************************************************************************************
 * @file   adc_manager.cc
 *
 * @brief  Source file defining the AdcManager class for the client
 *
 * @date   2025-04-24
 * @author Shahzaib Siddiqui
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <iostream>
#include <vector>

/* Inter-component Headers */
extern "C" {
#include "adc.h"
}

#include "command_code.h"

/* Intra-component Headers */
#include "adc_manager.h"
#include "app.h"

void AdcManager::setAdcRaw(std::string &payload) {
  m_adcDatagram.deserialize(payload);

  const uint8_t *receivedData = m_adcDatagram.getBuffer();

  uint16_t receivedReading = (static_cast<uint16_t>(receivedData[1U]) << 8) | receivedData[0U];

  GpioAddress pinAddress = { .port = static_cast<GpioPort>(m_adcDatagram.getGpioPort()), .pin = m_adcDatagram.getGpioPin() };

  adc_set_reading(&pinAddress, receivedReading);
}

void AdcManager::setAdcAllRaw(std::string &payload) {
  m_adcDatagram.deserialize(payload);

  const uint8_t *receivedData = m_adcDatagram.getBuffer();

  uint16_t receivedReading = (static_cast<uint16_t>(receivedData[1U]) << 8) | receivedData[0U];

  /* ADC Channels 1-4, 13-14 (C0-C3, C4,C5) */
  for (uint8_t i = 0U; i < 6U; i++) {
    GpioAddress pinAddress = { .port = GPIO_PORT_C, .pin = i };
    adc_set_reading(&pinAddress, receivedReading);
  }

  /* ADC Channels 5-12 (A0-A7) */
  for (uint8_t i = 0U; i < 8U; i++) {
    GpioAddress pinAddress = { .port = GPIO_PORT_A, .pin = i };
    adc_set_reading(&pinAddress, receivedReading);
  }

  /* ADC Channels 15-16 (B0-B1) */
  for (uint8_t i = 0U; i < 2U; i++) {
    GpioAddress pinAddress = { .port = GPIO_PORT_B, .pin = i };
    adc_set_reading(&pinAddress, receivedReading);
  }
}

std::string AdcManager::processReadAdcRaw(std::string &payload) {
  m_adcDatagram.deserialize(payload);

  GpioAddress pinAddress = { .port = static_cast<GpioPort>(m_adcDatagram.getGpioPort()), .pin = m_adcDatagram.getGpioPin() };

  uint16_t pinReading;
  adc_read_raw(&pinAddress, &pinReading);

  std::vector<uint8_t> byteArray;
  byteArray.push_back(static_cast<uint8_t>(pinReading & 0xFF));
  byteArray.push_back(static_cast<uint8_t>((pinReading >> 8) & 0xFF));

  m_adcDatagram.clearBuffer();
  m_adcDatagram.setBuffer(byteArray.data(), byteArray.size());

  return m_adcDatagram.serialize(CommandCode::ADC_GET_RAW);
}

std::string AdcManager::processReadAdcAllRaw() {
  std::vector<uint8_t> byteArray;
  uint16_t pinReading;

  /* ADC Channels 1-4 (C0-C3) */
  for (uint8_t i = 0U; i < 4U; i++) {
    GpioAddress pinAddress = { .port = GPIO_PORT_C, .pin = i };
    adc_read_raw(&pinAddress, &pinReading);

    byteArray.push_back(static_cast<uint8_t>(pinReading & 0xFF));
    byteArray.push_back(static_cast<uint8_t>((pinReading >> 8) & 0xFF));
  }

  /* ADC Channels 5-12 (A0-A7) */
  for (uint8_t i = 0U; i < 8U; i++) {
    GpioAddress pinAddress = { .port = GPIO_PORT_A, .pin = i };
    adc_read_raw(&pinAddress, &pinReading);

    byteArray.push_back(static_cast<uint8_t>(pinReading & 0xFF));
    byteArray.push_back(static_cast<uint8_t>((pinReading >> 8) & 0xFF));
  }

  /* ADC Channels 13-14 (C4-C5) */
  for (uint8_t i = 4U; i < 6U; i++) {
    GpioAddress pinAddress = { .port = GPIO_PORT_C, .pin = i };
    adc_read_raw(&pinAddress, &pinReading);

    byteArray.push_back(static_cast<uint8_t>(pinReading & 0xFF));
    byteArray.push_back(static_cast<uint8_t>((pinReading >> 8) & 0xFF));
  }

  /* ADC Channels 15-16 (B0-B1) */
  for (uint8_t i = 0U; i < 2U; i++) {
    GpioAddress pinAddress = { .port = GPIO_PORT_B, .pin = i };
    adc_read_raw(&pinAddress, &pinReading);

    byteArray.push_back(static_cast<uint8_t>(pinReading & 0xFF));
    byteArray.push_back(static_cast<uint8_t>((pinReading >> 8) & 0xFF));
  }

  m_adcDatagram.clearBuffer();
  m_adcDatagram.setBuffer(byteArray.data(), byteArray.size());

  return m_adcDatagram.serialize(CommandCode::ADC_GET_ALL_RAW);
}

std::string AdcManager::processReadAdcConverted(std::string &payload) {
  m_adcDatagram.deserialize(payload);

  GpioAddress pinAddress = { .port = static_cast<GpioPort>(m_adcDatagram.getGpioPort()), .pin = m_adcDatagram.getGpioPin() };

  uint16_t pinReading;
  adc_read_converted(&pinAddress, &pinReading);

  std::vector<uint8_t> byteArray;
  byteArray.push_back(static_cast<uint8_t>(pinReading & 0xFF));
  byteArray.push_back(static_cast<uint8_t>((pinReading >> 8) & 0xFF));

  m_adcDatagram.clearBuffer();
  m_adcDatagram.setBuffer(byteArray.data(), byteArray.size());

  return m_adcDatagram.serialize(CommandCode::ADC_GET_CONVERTED);
}

std::string AdcManager::processReadAdcAllConverted() {
  std::vector<uint8_t> byteArray;
  uint16_t pinReading;

  /* ADC Channels 1-4 (C0-C3) */
  for (uint8_t i = 0U; i < 4U; i++) {
    GpioAddress pinAddress = { .port = GPIO_PORT_C, .pin = i };
    adc_read_converted(&pinAddress, &pinReading);

    byteArray.push_back(static_cast<uint8_t>(pinReading & 0xFF));
    byteArray.push_back(static_cast<uint8_t>((pinReading >> 8) & 0xFF));
  }

  /* ADC Channels 5-12 (A0-A7) */
  for (uint8_t i = 0U; i < 8U; i++) {
    GpioAddress pinAddress = { .port = GPIO_PORT_A, .pin = i };
    adc_read_converted(&pinAddress, &pinReading);

    byteArray.push_back(static_cast<uint8_t>(pinReading & 0xFF));
    byteArray.push_back(static_cast<uint8_t>((pinReading >> 8) & 0xFF));
  }

  /* ADC Channels 13-14 (C4-C5) */
  for (uint8_t i = 4U; i < 6U; i++) {
    GpioAddress pinAddress = { .port = GPIO_PORT_C, .pin = i };
    adc_read_converted(&pinAddress, &pinReading);

    byteArray.push_back(static_cast<uint8_t>(pinReading & 0xFF));
    byteArray.push_back(static_cast<uint8_t>((pinReading >> 8) & 0xFF));
  }

  /* ADC Channels 15-16 (B0-B1) */
  for (uint8_t i = 0U; i < 2U; i++) {
    GpioAddress pinAddress = { .port = GPIO_PORT_B, .pin = i };
    adc_read_converted(&pinAddress, &pinReading);

    byteArray.push_back(static_cast<uint8_t>(pinReading & 0xFF));
    byteArray.push_back(static_cast<uint8_t>((pinReading >> 8) & 0xFF));
  }

  m_adcDatagram.clearBuffer();
  m_adcDatagram.setBuffer(byteArray.data(), byteArray.size());

  return m_adcDatagram.serialize(CommandCode::ADC_GET_ALL_CONVERTED);
}


