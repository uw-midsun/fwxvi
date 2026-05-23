/************************************************************************************************
 * @file   acs37800_datagram.cc
 *
 * @brief  Source file defining the acs37800 datagram class
 *
 * @date   2026-03-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

/* Inter-component Headers */

/* Intra-component Headers */
#include "acs37800_datagram.h"
#include "serialization.h"

namespace Datagram {

Acs37800::Acs37800(Payload &data) {
  m_acs37800Datagram = data;
}

std::string Acs37800::serialize(const CommandCode &commandCode) const {
  std::string serializedData;

  uint32_t voltage, current, active_power;
  std::memcpy(&voltage, &m_acs37800Datagram.voltage_mV, sizeof(float));
  std::memcpy(&current, &m_acs37800Datagram.current_amps, sizeof(float));
  std::memcpy(&active_power, &m_acs37800Datagram.active_power_mW, sizeof(float));

  serializeInteger<uint32_t>(serializedData, voltage);
  serializeInteger<uint32_t>(serializedData, current);
  serializeInteger<uint32_t>(serializedData, active_power);
  uint8_t status_flags = 0;

  if (m_acs37800Datagram.overcurrent_flag) status_flags |= MASK_FAULTOUT;       // Bit 1
  if (m_acs37800Datagram.overvoltage_flag) status_flags |= MASK_OVERVOLTAGE;    // Bit 3
  if (m_acs37800Datagram.undervoltage_flag) status_flags |= MASK_UNDERVOLTAGE;  // Bit 4

  serializeInteger<uint8_t>(serializedData, status_flags);
  return encodeCommand(commandCode, serializedData);
}

void Acs37800::deserialize(std::string &acs37800DatagramPayload) {
  std::size_t offset = 0;

  std::memcpy(&m_acs37800Datagram.voltage_mV, acs37800DatagramPayload.data() + offset, sizeof(float));
  offset += sizeof(float);

  std::memcpy(&m_acs37800Datagram.current_amps, acs37800DatagramPayload.data() + offset, sizeof(float));
  offset += sizeof(float);

  std::memcpy(&m_acs37800Datagram.active_power_mW, acs37800DatagramPayload.data() + offset, sizeof(float));
  offset += sizeof(float);

  uint8_t status_flags = deserializeInteger<uint8_t>(acs37800DatagramPayload, offset);
  m_acs37800Datagram.overcurrent_flag = ((status_flags & MASK_FAULTOUT) != 0);
  m_acs37800Datagram.overvoltage_flag = ((status_flags & MASK_OVERVOLTAGE) != 0);
  m_acs37800Datagram.undervoltage_flag = ((status_flags & MASK_UNDERVOLTAGE) != 0);
}

void Acs37800::setVoltage(float voltage_mV) {
  m_acs37800Datagram.voltage_mV = voltage_mV;
}

/**
 * @brief   Sets current
 * @param   current_amps Current to set as target
 */
void Acs37800::setCurrent(float current_amps) {
  m_acs37800Datagram.current_amps = current_amps;
}

/**
 * @brief   Sets active power
 * @param   active_power_mW Active power to set as target
 */
void Acs37800::setActivePower(float active_power_mW) {
  m_acs37800Datagram.active_power_mW = active_power_mW;
}

/**
 * @brief   Sets overcurrent flag
 * @param   overcurrent_flag Overcurrent flag to set as target
 */
void Acs37800::setOvercurrentFlag(bool overcurrent_flag) {
  m_acs37800Datagram.overcurrent_flag = overcurrent_flag;
}

/**
 * @brief   Sets overvoltage flag
 * @param   overvoltage_flag Overvoltage flag to set as target
 */
void Acs37800::setOvervoltageFlag(bool overvoltage_flag) {
  m_acs37800Datagram.overvoltage_flag = overvoltage_flag;
}

/**
 * @brief   Sets undervoltage flag
 * @param   undervoltage_flag Undervoltage flag to set as target
 */
void Acs37800::setUndervoltageFlag(bool undervoltage_flag) {
  m_acs37800Datagram.undervoltage_flag = undervoltage_flag;
}

float Acs37800::getVoltage() const {
  return m_acs37800Datagram.voltage_mV;
}

/**
 * @brief   Gets current in amps
 * @return  current in amps
 */
float Acs37800::getCurrent() const {
  return m_acs37800Datagram.current_amps;
}

/**
 * @brief   Gets active power in mW
 * @return  active power in mW
 */
float Acs37800::getActivePower() const {
  return m_acs37800Datagram.active_power_mW;
}

/**
 * @brief   Gets overcurrent flag
 * @return  Overcurrent flag
 */
bool Acs37800::getOvercurrentFlag() const {
  return m_acs37800Datagram.overcurrent_flag;
}

/**
 * @brief   Gets overvoltage flag
 * @return  Overvoltage flag
 */
bool Acs37800::getOvervoltageFlag() const {
  return m_acs37800Datagram.overvoltage_flag;
}

/**
 * @brief   Gets undervoltage flag
 * @return  Undervoltage flag
 */
bool Acs37800::getUndervoltageFlag() const {
  return m_acs37800Datagram.undervoltage_flag;
}

}  // namespace Datagram
