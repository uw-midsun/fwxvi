/* Standard library Headers */
#include <cstdint>
#include <stdexcept>

/* Inter-component Headers */
#include "command_code.h"

/* Intra-component Headers */
#include "app.h"
#include "adbms_afe_manager.h"
#include <iostream>


#define AFE_KEY "afe"

void AfeManager::loadAfeInfo(std::string &projectName){
  m_afeInfo = serverJSONManager.getProjectValue<std::unordered_map<std::string, std::string>>(projectName, AFE_KEY);
}

void AfeManager::saveAfeInfo(std::string &projectName){
  serverJSONManager.setProjectValue(projectName, AFE_KEY, m_afeInfo);

  /* Upon save, clear the memory */
  m_afeInfo.clear(); 
}

void AfeManager::updateAfeCellVoltage(std::string &projectName, std::string &payload){
  loadAfeInfo(projectName); 

  m_afeDatagram.deserialize(payload); 
  uint16_t index = m_afeDatagram.getIndex();
  uint8_t voltage = m_afeDatagram.getCellVoltage(index);
  
  m_afeInfo["cell" + std::to_string(index)] = std::to_string(voltage) + "mv";
  
  saveAfeInfo(projectName);
}

void AfeManager::updateAfeAuxVoltage(std::string &projectName, std::string &payload){
  loadAfeInfo(projectName); 

  m_afeDatagram.deserialize(payload); 
  uint16_t index = m_afeDatagram.getIndex();
  uint8_t voltage = m_afeDatagram.getAuxVoltage(index);
  
  m_afeInfo["aux" + std::to_string(index)] = std::to_string(voltage) + "mv";
  
  saveAfeInfo(projectName);
}

void AfeManager::updateAfeCellDevVoltage(std::string &projectName, std::string &payload){
  loadAfeInfo(projectName); 

  m_afeDatagram.deserialize(payload); 

  std::size_t dev_index = m_afeDatagram.getDevIndex(); 

  const uint16_t start = dev_index * 18;
  const uint16_t end   = start + 18;
  
  for (uint16_t cell = start; cell < end; ++cell){
    uint8_t voltage = m_afeDatagram.getCellVoltage(cell); 
    m_afeInfo["cell" + std::to_string(cell)] = std::to_string(voltage) + "mv"; 
  }
  
  saveAfeInfo(projectName); 
}

void AfeManager::updateAfeAuxDevVoltage(std::string &projectName, std::string &payload){
  loadAfeInfo(projectName); 

  m_afeDatagram.deserialize(payload);

  std::size_t dev_index = m_afeDatagram.getDevIndex(); 

  const uint16_t start = dev_index * 9; 
  const uint16_t end = start + 9; 

  for (uint16_t aux = start; aux < end; ++aux){
    uint8_t voltage = m_afeDatagram.getAuxVoltage(aux); 
    m_afeInfo["aux" + std::to_string(aux)] = std::to_string(voltage) + "mv"; 
  }

  saveAfeInfo(projectName); 
}

void AfeManager::updateAfeCellPackVoltage(std::string &projectName, std::string &payload){
  loadAfeInfo(projectName); 

  m_afeDatagram.deserialize(payload); 

  for (std::size_t dev_index = 0; dev_index < 3; ++dev_index){
    const uint16_t start = dev_index * 18;
    const uint16_t end   = start + 18;

    for (uint16_t cell = start; cell < end; ++cell){
      uint16_t voltage = m_afeDatagram.getCellVoltage(cell);
      m_afeInfo["cell" + std::to_string(cell)] = std::to_string(voltage) + "mv";
    }
  }
  saveAfeInfo(projectName); 
}

void AfeManager::updateAfeAuxPackVoltage(std::string &projectName, std::string &payload){
  loadAfeInfo(projectName); 

  m_afeDatagram.deserialize(payload); 

  for (std::size_t dev_index = 0; dev_index < 3; ++dev_index){
    const uint16_t start = dev_index * 9;
    const uint16_t end   = start + 9;

    for (uint16_t aux = start; aux < end; ++aux){
      uint16_t voltage = m_afeDatagram.getAuxVoltage(aux);
      m_afeInfo["aux" + std::to_string(aux)] = std::to_string(voltage) + "mv";
    }
  }
  saveAfeInfo(projectName); 
}

std::string AfeManager::createAfeCommand(CommandCode commandCode, std::string index, std::string data){
  try {
    switch(commandCode){
      /* Setters */
      case CommandCode::AFE_SET_CELL: {
        std::size_t idx = std::stoi(index);
        m_afeDatagram.setIndex(idx); 
        m_afeDatagram.setVoltage(static_cast<uint8_t>(std::stoi(data)));
        break;
      }
      case CommandCode::AFE_SET_AUX: {
        std::size_t idx = std::stoi(index); 
        m_afeDatagram.setIndex(idx);
        m_afeDatagram.setVoltage(static_cast<uint8_t>(std::stoi(data)));
        break;
      }

      case CommandCode::AFE_SET_DEV_CELL: {
        std::size_t idx = std::stoi(index);
        m_afeDatagram.setDeviceIndex(idx); 
        m_afeDatagram.setVoltage(static_cast<uint8_t>(std::stoi(data)));
        break;
      }
      case CommandCode::AFE_SET_DEV_AUX: {
        std::size_t idx = std::stoi(index); 
        m_afeDatagram.setDeviceIndex(idx);
        m_afeDatagram.setVoltage(static_cast<uint8_t>(std::stoi(data)));
        break;
      }

      case CommandCode::AFE_SET_PACK_AUX: 
      case CommandCode::AFE_SET_PACK_CELL: {
        m_afeDatagram.setVoltage(static_cast<uint8_t>(std::stoi(data)));
        break;
      }

      /* Getters */
      case CommandCode::AFE_GET_CELL: {
        std::size_t idx = std::stoi(index);
        m_afeDatagram.setIndex(idx);
        break;
      }
      case CommandCode::AFE_GET_AUX: {
        std::size_t idx = std::stoi(index); 
        m_afeDatagram.setIndex(idx);
        break;
      }
      case CommandCode::AFE_GET_DEV_CELL: {
        std::size_t idx = std::stoi(index);
        m_afeDatagram.setDeviceIndex(idx); 
        break;
      }
      case CommandCode::AFE_GET_DEV_AUX: {
        std::size_t idx = std::stoi(index); 
        m_afeDatagram.setDeviceIndex(idx);
        break;
      }
      case CommandCode::AFE_GET_PACK_CELL:
      case CommandCode::AFE_GET_PACK_AUX: {
        break;
      }

      default: {
        throw std::runtime_error("Invalid command code"); 
        break; 
      }
    }
      return m_afeDatagram.serialize(commandCode);
  }
  catch(const std::exception& e){
    std::cerr << "Afe Manager errror: " << e.what() << std::endl;
  }
  return "";
}
