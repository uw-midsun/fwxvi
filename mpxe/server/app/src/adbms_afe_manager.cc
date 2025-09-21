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
#define MAX_CELL_THERMISTORS_PER_DEVICE 16

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
  m_afeInfo["main_pack"]["cell_" + name] = std::to_string(voltage) + " mv";

  saveAfeInfo(projectName);
}

void AfeManager::updateAfeThermVoltage(std::string &projectName, std::string &payload) {
  loadAfeInfo(projectName);

  m_afeDatagram.deserialize(payload);
  uint16_t index = m_afeDatagram.getIndex();
  uint16_t voltage = m_afeDatagram.getThermVoltage(index);

  std::string name = (index < 10) ? "0" + std::to_string(index) : std::to_string(index);
  m_afeInfo["thermistor_temperature"]["thermistor_" + name] = std::to_string(voltage) + " mv";

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
    m_afeInfo["main_pack"]["cell_" + name] = std::to_string(voltage) + " mv";
  }

  saveAfeInfo(projectName);
}

void AfeManager::updateAfeThermDevVoltage(std::string &projectName, std::string &payload) {
  loadAfeInfo(projectName);

  m_afeDatagram.deserialize(payload);

  std::size_t dev_index = m_afeDatagram.getDevIndex();

  const uint16_t start = dev_index * MAX_CELL_THERMISTORS_PER_DEVICE;
  const uint16_t end = start + MAX_CELL_THERMISTORS_PER_DEVICE;

  for (uint16_t thermistor = start; thermistor < end; ++thermistor) {
    uint16_t voltage = m_afeDatagram.getThermVoltage(thermistor);

    std::string name = (thermistor < 10) ? "0" + std::to_string(thermistor) : std::to_string(thermistor);
    m_afeInfo["thermistor_temperature"]["thermistor_" + name] = std::to_string(voltage) + " mv";
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
      m_afeInfo["main_pack"]["cell_" + name] = std::to_string(voltage) + " mv";
    }
  }
  saveAfeInfo(projectName);
}

void AfeManager::updateAfeThermPackVoltage(std::string &projectName, std::string &payload) {
  loadAfeInfo(projectName);

  m_afeDatagram.deserialize(payload);

  for (std::size_t dev_index = 0; dev_index < 3; ++dev_index) {
    const uint16_t start = dev_index * MAX_CELL_THERMISTORS_PER_DEVICE;
    const uint16_t end = start + MAX_CELL_THERMISTORS_PER_DEVICE;

    for (uint16_t thermistor = start; thermistor < end; ++thermistor) {
      uint16_t voltage = m_afeDatagram.getThermVoltage(thermistor);

      std::string name = (thermistor < 10) ? "0" + std::to_string(thermistor) : std::to_string(thermistor);
      m_afeInfo["thermistor_temperature"]["thermistor_" + name] = std::to_string(voltage) + " mv";
    }
  }
  saveAfeInfo(projectName);
}

void AfeManager::updateAfeBoardThermVoltage(std::string &projectName, std::string &payload) {
  loadAfeInfo(projectName); 

  m_afeDatagram.deserialize(payload); 
  std::size_t dev_index = m_afeDatagram.getDevIndex(); 
  std::size_t board_therm_voltage = m_afeDatagram.getBoardThermVoltage(dev_index); 

  m_afeInfo["board_thermistors"]["board_" + std::to_string(dev_index)] = std::to_string(board_therm_voltage) + " mv"; 

  saveAfeInfo(projectName);
}

void AfeManager::updateAfeCellDischarge(std::string &projectName, std::string &payload) {
  loadAfeInfo(projectName);

  m_afeDatagram.deserialize(payload);

  uint8_t cell = m_afeDatagram.getIndex();
  bool is_discharge = m_afeDatagram.getCellDischarge(cell);

  std::string name = (cell < 10) ? "0" + std::to_string(cell) : std::to_string(cell);
  m_afeInfo["cell_discharge"]["cell_" + name] = (is_discharge) ? "on" : "off";

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
      m_afeInfo["cell_discharge"]["cell_" + name] = (is_discharge) ? "on" : "off";
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
      case CommandCode::AFE_SET_THERMISTOR: {
        uint8_t idx = static_cast<uint8_t>(std::stoi(index));
        uint16_t voltage = static_cast<uint16_t>(std::stoi(data));

        m_afeDatagram.setIndex(idx);
        m_afeDatagram.setThermVoltage(idx, voltage);
        break;
      }

      case CommandCode::AFE_SET_DEV_CELL: {
        std::size_t device_index = static_cast<std::size_t>(std::stoul(index));
        uint16_t voltage = static_cast<uint16_t>(std::stoi(data));

        m_afeDatagram.setDeviceIndex(device_index);
        m_afeDatagram.setDeviceCellVoltage(device_index, voltage);
        break;
      }
      case CommandCode::AFE_SET_DEV_THERMISTOR: {
        std::size_t device_index = static_cast<std::size_t>(std::stoul(index));
        uint16_t voltage = static_cast<uint16_t>(std::stoi(data));

        m_afeDatagram.setDeviceIndex(device_index);
        m_afeDatagram.setDeviceThermVoltage(device_index, voltage);
        break;
      }

      case CommandCode::AFE_SET_PACK_THERMISTOR: {
        uint16_t voltage = static_cast<uint16_t>(std::stoi(data));
        m_afeDatagram.setPackThermVoltage(voltage);
        break;
      }
      case CommandCode::AFE_SET_PACK_CELL: {
        uint16_t voltage = static_cast<uint16_t>(std::stoi(data));
        m_afeDatagram.setPackCellVoltage(voltage);
        break;
      }

      case CommandCode::AFE_SET_BOARD_TEMP: {
        uint16_t voltage = static_cast<uint16_t>(std::stoi(data)); 
        std::size_t dev_index = static_cast<std::size_t>(std::stoul(index)); 
        
        m_afeDatagram.setBoardTherm(dev_index, voltage); 
        m_afeDatagram.setDeviceIndex(dev_index); 
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
      case CommandCode::AFE_GET_THERMISTOR: {
        uint8_t idx = static_cast<uint8_t>(std::stoi(index));
        m_afeDatagram.setIndex(idx);
        break;
      }
      case CommandCode::AFE_GET_DEV_CELL: {
        std::size_t idx = static_cast<std::size_t>(std::stoul(index));
        m_afeDatagram.setDeviceIndex(idx);
        break;
      }
      case CommandCode::AFE_GET_DEV_THERMISTOR: {
        std::size_t idx = static_cast<std::size_t>(std::stoul(index));
        m_afeDatagram.setDeviceIndex(idx);
        break;
      }
      case CommandCode::AFE_GET_PACK_CELL:
      case CommandCode::AFE_GET_PACK_THERMISTOR: {
        break;
      }
      case CommandCode::AFE_GET_BOARD_TEMP: {
        std::size_t idx = static_cast<std::size_t>(std::stoul(index)); 
        m_afeDatagram.setDeviceIndex(idx); 
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
