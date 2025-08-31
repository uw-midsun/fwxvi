/************************************************************************************************
 * @file   adbms_afe_manager.cc
 *
 * @brief  Source file defining the AfeManager Class for the server
 *
 * @date   2025-07-22
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <stdexcept>

/* Inter-component Headers */
#include "command_code.h"

/* Intra-component Headers */
#include <iostream>

#include "adbms_afe_manager.h"
#include "app.h"

#define AFE_KEY "afe"
#define MAX_CELLS_PER_DEVICE 18
#define MAX_AUX_PER_DEVICE 9

void AfeManager::loadAfeInfo(std::string &projectName) {
  m_afeInfo = serverJSONManager.getProjectValue<std::unordered_map<std::string, AfeManager::AfeObjectInfo>>(projectName, AFE_KEY);
}

void AfeManager::saveAfeInfo(std::string &projectName) {
  serverJSONManager.setProjectValue(projectName, AFE_KEY, m_afeInfo);

  /* Upon save, clear the memory */
  m_afeInfo.clear();
}

void AfeManager::updateAfeCellVoltage(std::string &projectName, std::string &payload) {
  loadAfeInfo(projectName);

  m_afeDatagram.deserialize(payload);
  uint16_t index = m_afeDatagram.getIndex();
  uint16_t voltage = m_afeDatagram.getCellVoltage(index);

  std::string name = (index < 10) ? "0" + std::to_string(index) : std::to_string(index);
  m_afeInfo["main_pack"]["cell" + name] = std::to_string(voltage) + " mv";

  saveAfeInfo(projectName);
}

void AfeManager::updateAfeAuxVoltage(std::string &projectName, std::string &payload) {
  loadAfeInfo(projectName);

  m_afeDatagram.deserialize(payload);
  uint16_t index = m_afeDatagram.getIndex();
  uint16_t voltage = m_afeDatagram.getAuxVoltage(index);

  std::string name = (index < 10) ? "0" + std::to_string(index) : std::to_string(index);
  m_afeInfo["thermistor_temperature"]["aux" + name] = std::to_string(voltage) + " mv";

  saveAfeInfo(projectName);
}

void AfeManager::updateAfeCellDevVoltage(std::string &projectName, std::string &payload) {
  loadAfeInfo(projectName);

  m_afeDatagram.deserialize(payload);

  std::size_t dev_index = m_afeDatagram.getDevIndex();

  const uint16_t start = dev_index * MAX_CELLS_PER_DEVICE;
  const uint16_t end = start + MAX_CELLS_PER_DEVICE;

  for (uint16_t cell = start; cell < end; ++cell) {
    uint16_t voltage = m_afeDatagram.getCellVoltage(cell);

    std::string name = (cell < 10) ? "0" + std::to_string(cell) : std::to_string(cell);
    m_afeInfo["main_pack"]["cell" + name] = std::to_string(voltage) + " mv";
  }

  saveAfeInfo(projectName);
}

void AfeManager::updateAfeAuxDevVoltage(std::string &projectName, std::string &payload) {
  loadAfeInfo(projectName);

  m_afeDatagram.deserialize(payload);

  std::size_t dev_index = m_afeDatagram.getDevIndex();

  const uint16_t start = dev_index * MAX_AUX_PER_DEVICE;
  const uint16_t end = start + MAX_AUX_PER_DEVICE;

  for (uint16_t aux = start; aux < end; ++aux) {
    uint16_t voltage = m_afeDatagram.getAuxVoltage(aux);

    std::string name = (aux < 10) ? "0" + std::to_string(aux) : std::to_string(aux);
    m_afeInfo["thermistor_temperature"]["aux" + name] = std::to_string(voltage) + " mv";
  }

  saveAfeInfo(projectName);
}

void AfeManager::updateAfeCellPackVoltage(std::string &projectName, std::string &payload) {
  loadAfeInfo(projectName);

  m_afeDatagram.deserialize(payload);

  for (std::size_t dev_index = 0; dev_index < 3; ++dev_index) {
    const uint16_t start = dev_index * MAX_CELLS_PER_DEVICE;
    const uint16_t end = start + MAX_CELLS_PER_DEVICE;

    for (uint16_t cell = start; cell < end; ++cell) {
      uint16_t voltage = m_afeDatagram.getCellVoltage(cell);

      std::string name = (cell < 10) ? "0" + std::to_string(cell) : std::to_string(cell);
      m_afeInfo["main_pack"]["cell" + name] = std::to_string(voltage) + " mv";
    }
  }
  saveAfeInfo(projectName);
}

void AfeManager::updateAfeAuxPackVoltage(std::string &projectName, std::string &payload) {
  loadAfeInfo(projectName);

  m_afeDatagram.deserialize(payload);

  for (std::size_t dev_index = 0; dev_index < 3; ++dev_index) {
    const uint16_t start = dev_index * MAX_AUX_PER_DEVICE;
    const uint16_t end = start + MAX_AUX_PER_DEVICE;

    for (uint16_t aux = start; aux < end; ++aux) {
      uint16_t voltage = m_afeDatagram.getAuxVoltage(aux);

      std::string name = (aux < 10) ? "0" + std::to_string(aux) : std::to_string(aux);
      m_afeInfo["thermistor_temperature"]["aux" + name] = std::to_string(voltage) + " mv";
    }
  }
  saveAfeInfo(projectName);
}

void AfeManager::updateAfeCellDischarge(std::string &projectName, std::string &payload) {
  loadAfeInfo(projectName);

  m_afeDatagram.deserialize(payload);

  uint8_t cell = m_afeDatagram.getIndex();
  bool is_discharge = m_afeDatagram.getCellDischarge(cell);

  std::string name = (cell < 10) ? "0" + std::to_string(cell) : std::to_string(cell);
  m_afeInfo["cell_discharge"]["cell" + name] = (is_discharge) ? "on" : "off";

  saveAfeInfo(projectName);
}

void AfeManager::updateAfeCellPackDischarge(std::string &projectName, std::string &payload) {
  loadAfeInfo(projectName);

  m_afeDatagram.deserialize(payload);

  for (std::size_t dev_index = 0; dev_index < 3; ++dev_index) {
    const uint16_t start = dev_index * MAX_CELLS_PER_DEVICE;
    const uint16_t end = start + MAX_CELLS_PER_DEVICE;

    for (uint16_t cell = start; cell < end; ++cell) {
      bool is_discharge = m_afeDatagram.getCellDischarge(cell);

      std::string name = (cell < 10) ? "0" + std::to_string(cell) : std::to_string(cell);
      m_afeInfo["cell_discharge"]["cell" + name] = (is_discharge) ? "on" : "off";
    }
  }
  saveAfeInfo(projectName);
}

std::string AfeManager::createAfeCommand(CommandCode commandCode, std::string index, std::string data) {
  try {
    switch (commandCode) {
      /* Setters */
      case CommandCode::AFE_SET_CELL: {
        uint8_t idx = static_cast<uint8_t>(std::stoi(index));
        uint16_t voltage = static_cast<uint16_t>(std::stoi(data));

        m_afeDatagram.setIndex(idx);
        m_afeDatagram.setCellVoltage(idx, voltage);
        break;
      }
      case CommandCode::AFE_SET_AUX: {
        uint8_t idx = static_cast<uint8_t>(std::stoi(index));
        uint16_t voltage = static_cast<uint16_t>(std::stoi(data));

        m_afeDatagram.setIndex(idx);
        m_afeDatagram.setAuxVoltage(idx, voltage);
        break;
      }

      case CommandCode::AFE_SET_DEV_CELL: {
        std::size_t device_index = static_cast<std::size_t>(std::stoul(index));
        uint16_t voltage = static_cast<uint16_t>(std::stoi(data));

        m_afeDatagram.setDeviceIndex(device_index);
        m_afeDatagram.setDeviceCellVoltage(device_index, voltage);
        break;
      }
      case CommandCode::AFE_SET_DEV_AUX: {
        std::size_t device_index = static_cast<std::size_t>(std::stoul(index));
        uint16_t voltage = static_cast<uint16_t>(std::stoi(data));

        m_afeDatagram.setDeviceIndex(device_index);
        m_afeDatagram.setDeviceAuxVoltage(device_index, voltage);
        break;
      }

      case CommandCode::AFE_SET_PACK_AUX: {
        uint16_t voltage = static_cast<uint16_t>(std::stoi(data));
        m_afeDatagram.setPackAuxVoltage(voltage);
        break;
      }
      case CommandCode::AFE_SET_PACK_CELL: {
        uint16_t voltage = static_cast<uint16_t>(std::stoi(data));
        m_afeDatagram.setPackCellVoltage(voltage);
        break;
      }
      case CommandCode::AFE_SET_DISCHARGE: {
        bool is_discharge;

        if (data == "ON" || data == "on") {
          is_discharge = static_cast<bool>(Datagram::ADBMS_AFE::DischargeState::DISCHARGE_ON);
        } else if (data == "OFF" || data == "off") {
          is_discharge = static_cast<bool>(Datagram::ADBMS_AFE::DischargeState::DISCHARGE_OFF);
        } else {
          throw std::runtime_error("Invalid Discharge state: " + data);
          break;
        }

        uint8_t idx = static_cast<uint8_t>(std::stoi(index));
        m_afeDatagram.setIndex(idx);
        m_afeDatagram.setCellDischarge(is_discharge, idx);

        break;
      }
      case CommandCode::AFE_SET_PACK_DISCHARGE: {
        bool is_discharge;

        if (data == "ON" || data == "on") {
          is_discharge = static_cast<bool>(Datagram::ADBMS_AFE::DischargeState::DISCHARGE_ON);
        } else if (data == "OFF" || data == "off") {
          is_discharge = static_cast<bool>(Datagram::ADBMS_AFE::DischargeState::DISCHARGE_OFF);
        } else {
          throw std::runtime_error("Invalid Discharge state: " + data);
          break;
        }

        m_afeDatagram.setCellPackDischarge(is_discharge);

        break;
      }

      /* Getters */
      case CommandCode::AFE_GET_CELL: {
        uint8_t idx = static_cast<uint8_t>(std::stoi(index));
        m_afeDatagram.setIndex(idx);
        break;
      }
      case CommandCode::AFE_GET_AUX: {
        uint8_t idx = static_cast<uint8_t>(std::stoi(index));
        m_afeDatagram.setIndex(idx);
        break;
      }
      case CommandCode::AFE_GET_DEV_CELL: {
        std::size_t idx = static_cast<std::size_t>(std::stoul(index));
        m_afeDatagram.setDeviceIndex(idx);
        break;
      }
      case CommandCode::AFE_GET_DEV_AUX: {
        std::size_t idx = static_cast<std::size_t>(std::stoul(index));
        m_afeDatagram.setDeviceIndex(idx);
        break;
      }
      case CommandCode::AFE_GET_PACK_CELL:
      case CommandCode::AFE_GET_PACK_AUX: {
        break;
      }
      case CommandCode::AFE_GET_DISCHARGE: {
        uint8_t idx = static_cast<uint8_t>(std::stoi(index));
        m_afeDatagram.setIndex(idx);
        break;
      }
      case CommandCode::AFE_GET_PACK_DISCHARGE: {
        break;
      }
      default: {
        throw std::runtime_error("Invalid command code");
        break;
      }
    }
    return m_afeDatagram.serialize(commandCode);
  } catch (const std::exception &e) {
    std::cerr << "Afe Manager errror: " << e.what() << std::endl;
  }
  return "";
}
