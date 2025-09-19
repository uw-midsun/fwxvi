/************************************************************************************************
 * @file   adbms_afe_manager.cc
 *
 * @brief  Source file defining the AfeManager class for the client
 *
 * @date   2025-07-22
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <iostream>
#include <vector>

/* Inter-component Headers */
extern "C" {
#include "adbms_afe.h"
}

#include "command_code.h"

/* Instra-componenet Headers */
#include "adbms_afe_manager.h"
#include "app.h"

/* Constructor */
#include <iomanip>

/* SET
-------------------------------------------------*/
void AfeManager::setAfeCell(std::string &payload) {
  m_afeDatagram.deserialize(payload);

  uint8_t cell_index = m_afeDatagram.getIndex();

  if (cell_index >= ADBMS_AFE_MAX_CELLS) {
    std::cout << "Invalid Index" << std::endl;
    return;
  }

  uint16_t voltage = m_afeDatagram.getCellVoltage(cell_index);

  AdbmsAfeStorage *p_afe = adbms_afe_get_storage();

  if (p_afe != NULL) {
    adbms_afe_set_cell_voltage(p_afe, cell_index, voltage);
  }
}

void AfeManager::setAfeAux(std::string &payload) {
  m_afeDatagram.deserialize(payload);

  uint8_t aux_index = m_afeDatagram.getIndex();

  if (aux_index >= ADBMS_AFE_MAX_CELL_THERMISTORS) {
    std::cout << "Invalid Index" << std::endl;
    return;
  }

  uint16_t voltage = m_afeDatagram.getAuxVoltage(aux_index);

  AdbmsAfeStorage *p_afe = adbms_afe_get_storage();

  if (p_afe != NULL) {
    /* TODO: Update the entire MPXE toolchain to use new ADBMS API */
    adbms_afe_set_thermistor_voltage(p_afe, 0, aux_index, voltage);
  }
}

void AfeManager::setAfeDevCell(std::string &payload) {
  m_afeDatagram.deserialize(payload);

  std::size_t device_index = m_afeDatagram.getDevIndex();

  if (device_index >= ADBMS_AFE_MAX_DEVICES) {
    std::cout << "Invalid Index" << std::endl;
    return;
  }
  std::size_t cache_index = static_cast<std::size_t>(Datagram::ADBMS_AFE::CacheIndex::CELL_DEV_0) + device_index;
  uint16_t voltage = m_afeDatagram.getCache(static_cast<Datagram::ADBMS_AFE::CacheIndex>(cache_index));

  AdbmsAfeStorage *p_afe = adbms_afe_get_storage();

  if (p_afe != NULL) {
    adbms_afe_set_afe_dev_cell_voltages(p_afe, device_index, voltage);
  }
}

void AfeManager::setAfeDevAux(std::string &payload) {
  m_afeDatagram.deserialize(payload);

  std::size_t device_index = m_afeDatagram.getDevIndex();

  if (device_index >= ADBMS_AFE_MAX_DEVICES) {
    std::cout << "Invalid Index" << std::endl;
    return;
  }

  std::size_t cache_index = static_cast<std::size_t>(Datagram::ADBMS_AFE::CacheIndex::AUX_DEV_0) + device_index;
  uint16_t voltage = m_afeDatagram.getCache(static_cast<Datagram::ADBMS_AFE::CacheIndex>(cache_index));

  AdbmsAfeStorage *p_afe = adbms_afe_get_storage();

  if (p_afe != NULL) {
    adbms_afe_set_afe_dev_thermistor_voltages(p_afe, device_index, voltage);
  }
}

void AfeManager::setAfePackCell(std::string &payload) {
  m_afeDatagram.deserialize(payload);

  uint16_t voltage = m_afeDatagram.getCache(Datagram::ADBMS_AFE::CacheIndex::CELL_PACK);

  AdbmsAfeStorage *p_afe = adbms_afe_get_storage();

  if (p_afe != NULL) {
    adbms_afe_set_pack_cell_voltages(p_afe, voltage);
  }
}

void AfeManager::setAfePackAux(std::string &payload) {
  m_afeDatagram.deserialize(payload);

  uint16_t voltage = m_afeDatagram.getCache(Datagram::ADBMS_AFE::CacheIndex::AUX_PACK);

  AdbmsAfeStorage *p_afe = adbms_afe_get_storage();

  if (p_afe != NULL) {
    adbms_afe_set_pack_thermistor_voltages(p_afe, voltage);
  }
}

void AfeManager::setCellDischarge(std::string &payload) {
  m_afeDatagram.deserialize(payload);

  uint8_t cell_index = m_afeDatagram.getIndex();
  bool is_discharge = m_afeDatagram.getCellDischarge(cell_index);

  AdbmsAfeStorage *p_afe = adbms_afe_get_storage();

  if (p_afe != NULL) {
    adbms_afe_toggle_cell_discharge(p_afe, cell_index, is_discharge);
  }
}

void AfeManager::setCellPackDischarge(std::string &payload) {
  m_afeDatagram.deserialize(payload);

  bool is_discharge = static_cast<bool>(m_afeDatagram.getCache(Datagram::ADBMS_AFE::CacheIndex::DISCHARGE_PACK));

  AdbmsAfeStorage *p_afe = adbms_afe_get_storage();

  for (uint8_t cell_index = 0; cell_index < ADBMS_AFE_MAX_CELLS; ++cell_index) {
    if (p_afe != NULL) {
      adbms_afe_toggle_cell_discharge(p_afe, cell_index, is_discharge);
    }
  }
}

/* PROCESS
-------------------------------------------------------- */
std::string AfeManager::processAfeCell(std::string &payload) {
  m_afeDatagram.deserialize(payload);

  uint16_t cell_index = m_afeDatagram.getIndex();

  uint16_t voltage = 0;

  AdbmsAfeStorage *p_afe = adbms_afe_get_storage();

  if (p_afe != NULL) {
    voltage = adbms_afe_get_cell_voltage(p_afe, cell_index);
  }

  m_afeDatagram.setCellVoltage(cell_index, voltage);

  return m_afeDatagram.serialize(CommandCode::AFE_GET_CELL);
}

std::string AfeManager::processAfeAux(std::string &payload) {
  m_afeDatagram.deserialize(payload);

  uint16_t aux_index = m_afeDatagram.getIndex();

  uint16_t voltage = 0;

  AdbmsAfeStorage *p_afe = adbms_afe_get_storage();

  if (p_afe != NULL) {
    /* TODO: Update the entire MPXE toolchain to use new ADBMS API */
    voltage = adbms_afe_get_thermistor_voltage(p_afe, 0, aux_index);
  }

  m_afeDatagram.setAuxVoltage(aux_index, voltage);

  return m_afeDatagram.serialize(CommandCode::AFE_GET_AUX);
}

std::string AfeManager::processAfeDevCell(std::string &payload) {
  m_afeDatagram.deserialize(payload);

  std::size_t dev_index = m_afeDatagram.getDevIndex();
  const uint16_t start = dev_index * ADBMS_AFE_MAX_CELLS_PER_DEVICE;
  const uint16_t end = start + ADBMS_AFE_MAX_CELLS_PER_DEVICE;

  AdbmsAfeStorage *p_afe = adbms_afe_get_storage();

  for (uint16_t cell = start; cell < end; ++cell) {
    uint16_t voltage = 0;

    if (p_afe != NULL) {
      voltage = adbms_afe_get_cell_voltage(p_afe, cell);
    }

    m_afeDatagram.setCellVoltage(cell, voltage);
  }

  return m_afeDatagram.serialize(CommandCode::AFE_GET_DEV_CELL);
}
std::string AfeManager::processAfeDevAux(std::string &payload) {
  m_afeDatagram.deserialize(payload);

  std::size_t dev_index = m_afeDatagram.getDevIndex();

  AdbmsAfeStorage *p_afe = adbms_afe_get_storage();

  for (uint16_t thermistor_index = 0; thermistor_index < ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE; ++thermistor_index) {
    uint16_t voltage = 0;

    if (p_afe != NULL) {
      /* TODO: Update the entire MPXE toolchain to use new ADBMS API */
      voltage = adbms_afe_get_thermistor_voltage(p_afe, dev_index, thermistor_index);
    }

    m_afeDatagram.setAuxVoltage(thermistor_index, voltage);
  }

  return m_afeDatagram.serialize(CommandCode::AFE_GET_DEV_AUX);
}

std::string AfeManager::processAfePackCell() {
  AdbmsAfeStorage *p_afe = adbms_afe_get_storage();

  for (std::size_t dev_index = 0; dev_index < ADBMS_AFE_MAX_DEVICES; ++dev_index) {
    const uint16_t start = dev_index * ADBMS_AFE_MAX_CELLS_PER_DEVICE;
    const uint16_t end = start + ADBMS_AFE_MAX_CELLS_PER_DEVICE;

    for (uint16_t cell = start; cell < end; ++cell) {
      uint16_t voltage = 0;

      if (p_afe != NULL) {
        voltage = adbms_afe_get_cell_voltage(p_afe, cell);
      }

      m_afeDatagram.setCellVoltage(cell, voltage);
    }
  }

  return m_afeDatagram.serialize(CommandCode::AFE_GET_PACK_CELL);
}
std::string AfeManager::processAfePackAux() {
  AdbmsAfeStorage *p_afe = adbms_afe_get_storage();

  for (std::size_t dev_index = 0; dev_index < ADBMS_AFE_MAX_DEVICES; ++dev_index) {
    for (uint16_t thermistor_index = 0; thermistor_index < ADBMS_AFE_MAX_CELL_THERMISTORS_PER_DEVICE; ++thermistor_index) {
      uint16_t voltage = 0;

      if (p_afe != NULL) {
        /* TODO: Update the entire MPXE toolchain to use new ADBMS API */
        voltage = adbms_afe_get_thermistor_voltage(p_afe, dev_index, thermistor_index);
      }

      m_afeDatagram.setAuxVoltage(thermistor_index, voltage);
    }
  }

  return m_afeDatagram.serialize(CommandCode::AFE_GET_PACK_AUX);
}

std::string AfeManager::processCellDischarge(std::string &payload) {
  m_afeDatagram.deserialize(payload);
  uint16_t cell_index = m_afeDatagram.getIndex();

  bool is_discharge = false;

  AdbmsAfeStorage *p_afe = adbms_afe_get_storage();

  if (p_afe != NULL) {
    adbms_afe_get_cell_discharge(p_afe, cell_index);
  }

  m_afeDatagram.setCellDischarge(is_discharge, cell_index);

  return m_afeDatagram.serialize(CommandCode::AFE_GET_DISCHARGE);
}

std::string AfeManager::processCellPackDischarge() {
  AdbmsAfeStorage *p_afe = adbms_afe_get_storage();

  for (std::size_t dev_index = 0; dev_index < ADBMS_AFE_MAX_DEVICES; ++dev_index) {
    const uint16_t start = dev_index * ADBMS_AFE_MAX_CELLS_PER_DEVICE;
    const uint16_t end = start + ADBMS_AFE_MAX_CELLS_PER_DEVICE;

    for (uint16_t cell = start; cell < end; ++cell) {
      bool is_discharge = false;

      if (p_afe != NULL) {
        adbms_afe_get_cell_discharge(p_afe, cell);
      }

      m_afeDatagram.setCellDischarge(is_discharge, cell);
    }
  }

  return m_afeDatagram.serialize(CommandCode::AFE_GET_PACK_DISCHARGE);
}
