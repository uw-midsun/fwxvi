/************************************************************************************************
 * @file   afe_datagram.cc
 *
 * @brief  Source file defining the afe datagram class
 *
 * @date   2025-07-22
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

/* Inter-component Headers */

/* Intra-component Headers */
#include "adbms_afe_datagram.h"
#include "serialization.h"

namespace Datagram {
ADBMS_AFE::ADBMS_AFE(Payload &data) {
  m_afeDatagram = data;
}

std::string ADBMS_AFE::serialize(const CommandCode &commandCode) const {
  std::string serializedData;

  serializeInteger<uint8_t>(serializedData, m_afeDatagram.index);
  serializeInteger<std::size_t>(serializedData, m_afeDatagram.dev_index);

  serializeInteger<uint8_t>(serializedData, AFE_MAX_CELLS * sizeof(uint16_t));
  serializedData.append(reinterpret_cast<const char *>(m_afeDatagram.cell_voltages), AFE_MAX_CELLS * sizeof(uint16_t));

  serializeInteger<uint8_t>(serializedData, AFE_MAX_CELL_THERMISTORS * sizeof(uint16_t));
  serializedData.append(reinterpret_cast<const char *>(m_afeDatagram.therm_voltages), AFE_MAX_CELL_THERMISTORS * sizeof(uint16_t));

  serializeInteger<uint8_t>(serializedData, AFE_MAX_BOARD_THERMISTORS * sizeof(uint16_t));
  serializedData.append(reinterpret_cast<const char *>(m_afeDatagram.board_therm_voltages), AFE_MAX_BOARD_THERMISTORS * sizeof(uint16_t));

  serializeInteger<uint8_t>(serializedData, static_cast<uint8_t>(AFE_MAX_CELLS));
  serializedData.append(reinterpret_cast<const char *>(m_afeDatagram.cell_discharges), AFE_MAX_CELLS);

  serializeInteger<uint8_t>(serializedData, CACHE_SIZE * sizeof(uint16_t));
  serializedData.append(reinterpret_cast<const char *>(m_afeDatagram.cache), CACHE_SIZE * sizeof(uint16_t));

  return encodeCommand(commandCode, serializedData);
}

void ADBMS_AFE::deserialize(std::string &afeDatagramPayload) {
  std::size_t offset = 0;

  m_afeDatagram.index = deserializeInteger<uint8_t>(afeDatagramPayload, offset);
  m_afeDatagram.dev_index = deserializeInteger<std::size_t>(afeDatagramPayload, offset);

  deserializeInteger<uint8_t>(afeDatagramPayload, offset);
  std::memcpy(m_afeDatagram.cell_voltages, afeDatagramPayload.data() + offset, AFE_MAX_CELLS * sizeof(uint16_t));
  offset += AFE_MAX_CELLS * sizeof(uint16_t);

  deserializeInteger<uint8_t>(afeDatagramPayload, offset);
  std::memcpy(m_afeDatagram.therm_voltages, afeDatagramPayload.data() + offset, AFE_MAX_CELL_THERMISTORS * sizeof(uint16_t));
  offset += AFE_MAX_CELL_THERMISTORS * sizeof(uint16_t);

  deserializeInteger<uint8_t>(afeDatagramPayload, offset);
  std::memcpy(m_afeDatagram.board_therm_voltages, afeDatagramPayload.data() + offset, AFE_MAX_BOARD_THERMISTORS * sizeof(uint16_t));
  offset += AFE_MAX_BOARD_THERMISTORS * sizeof(uint16_t);

  deserializeInteger<uint8_t>(afeDatagramPayload, offset);  // length (ignored or validated)
  std::memcpy(m_afeDatagram.cell_discharges, afeDatagramPayload.data() + offset, AFE_MAX_CELLS);
  offset += AFE_MAX_CELLS;

  deserializeInteger<uint8_t>(afeDatagramPayload, offset);
  std::memcpy(m_afeDatagram.cache, afeDatagramPayload.data() + offset, CACHE_SIZE * sizeof(uint16_t));
  offset += CACHE_SIZE;
}

/* SETTERS
--------------------------------------------*/
void ADBMS_AFE::setIndex(uint8_t new_index) {
  m_afeDatagram.index = new_index;
}

void ADBMS_AFE::setDeviceIndex(std::size_t new_index) {
  m_afeDatagram.dev_index = new_index;
}

void ADBMS_AFE::setCellVoltage(uint8_t index, uint16_t voltage) {
  if (index >= AFE_MAX_CELLS) {
    std::cout << "Invalid Index" << std::endl;
    return;
  }

  m_afeDatagram.cell_voltages[index] = voltage;
}

void ADBMS_AFE::setThermVoltage(uint8_t index, uint16_t voltage) {
  if (index >= AFE_MAX_BOARD_THERMISTORS) {
    std::cout << "Invalid Index" << std::endl;
    return;
  }

  m_afeDatagram.therm_voltages[index] = voltage;
}

void ADBMS_AFE::setDeviceCellVoltage(std::size_t dev_index, uint16_t voltage) {
  uint8_t start = dev_index * AFE_MAX_CELLS_PER_DEVICE;
  for (uint8_t i = 0; i < AFE_MAX_CELLS_PER_DEVICE; ++i) {
    setCellVoltage((start + i), voltage);
  }

  std::size_t idx = static_cast<std::size_t>(CacheIndex::CELL_DEV_0) + dev_index;
  CacheIndex cache_index = static_cast<CacheIndex>(idx);
  setCache(cache_index, voltage);
}

void ADBMS_AFE::setDeviceThermVoltage(std::size_t dev_index, uint16_t voltage) {
  uint8_t start = dev_index * AFE_MAX_CELL_THERMISTORS_PER_DEVICE;
  for (uint8_t i = 0; i < AFE_MAX_CELL_THERMISTORS_PER_DEVICE; ++i) {
    setThermVoltage((start + i), voltage);
  }

  std::size_t idx = static_cast<std::size_t>(CacheIndex::THERMISTOR_DEV_0) + dev_index;
  CacheIndex cache_index = static_cast<CacheIndex>(idx);
  setCache(cache_index, voltage);
}

void ADBMS_AFE::setPackCellVoltage(uint16_t voltage) {
  for (uint8_t i = 0; i < AFE_MAX_CELLS; ++i) {
    setCellVoltage((i), voltage);
  }
  setCache(CacheIndex::CELL_PACK, voltage);
}

void ADBMS_AFE::setPackThermVoltage(uint16_t voltage) {
  for (uint8_t i = 0; i < AFE_MAX_CELL_THERMISTORS; ++i) {
    setThermVoltage((i), voltage);
  }
  setCache(CacheIndex::THERMISTOR_PACK, voltage);
}

void ADBMS_AFE::setCellDischarge(bool is_discharge, uint8_t cell_index) {
  if (cell_index >= AFE_MAX_CELLS) {
    std::cout << "Invalid Index" << std::endl;
    return;
  }
  m_afeDatagram.cell_discharges[cell_index] = is_discharge;
}

void ADBMS_AFE::setBoardTherm(std::size_t dev_index, uint16_t voltage) {
  if (dev_index >= AFE_MAX_BOARD_THERMISTORS) {
    std::cout << "Invalid index" << std::endl;
    return;
  }
  m_afeDatagram.board_therm_voltages[dev_index] = voltage;
}

void ADBMS_AFE::setCellPackDischarge(bool is_discharge) {
  for (uint8_t i = 0; i < AFE_MAX_CELL_THERMISTORS; ++i) {
    setCellDischarge(is_discharge, i);
  }

  setCache(CacheIndex::DISCHARGE_PACK, static_cast<uint16_t>(is_discharge));
}

void ADBMS_AFE::setCache(CacheIndex cache_index, uint16_t value) {
  m_afeDatagram.cache[static_cast<std::size_t>(cache_index)] = value;
}

/* GETTERS
--------------------------------------------*/
uint8_t ADBMS_AFE::getIndex() const {
  return m_afeDatagram.index;
}

std::size_t ADBMS_AFE::getDevIndex() const {
  return m_afeDatagram.dev_index;
}

uint16_t ADBMS_AFE::getCellVoltage(std::size_t index) const {
  if (index < AFE_MAX_CELLS) return m_afeDatagram.cell_voltages[index];

  std::cout << "Invalid Index" << std::endl;
  return -1;
}

uint16_t ADBMS_AFE::getThermVoltage(std::size_t index) const {
  if (index < AFE_MAX_CELL_THERMISTORS) return m_afeDatagram.therm_voltages[index];
  std::cout << "Invalid Index" << std::endl;
  return -1;
}

uint16_t ADBMS_AFE::getBoardThermVoltage(std::size_t dev_index) const {
  if (dev_index < AFE_MAX_BOARD_THERMISTORS) return m_afeDatagram.board_therm_voltages[dev_index];
  std::cout << "Invalid index" << std::endl;
  return -1;
}

bool ADBMS_AFE::getCellDischarge(uint8_t cell_index) const {
  if (cell_index < AFE_MAX_CELLS) return m_afeDatagram.cell_discharges[cell_index];
  std::cout << "Invalid Index" << std::endl;
  return -1;
}

uint16_t ADBMS_AFE::getCache(CacheIndex cache_index) const {
  return m_afeDatagram.cache[static_cast<std::size_t>(cache_index)];
}

}  // namespace Datagram
