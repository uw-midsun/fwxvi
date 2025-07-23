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
  uint16_t voltage = m_afeDatagram.getVoltage(); 
  uint16_t index = m_afeDatagram.getIndex();
  //uint16_t voltage = m_afeDatagram.getCellVoltage(index);
  
  m_afeInfo["cell" + std::to_string(index)] = std::to_string(voltage) + "mv";
  
  saveAfeInfo(projectName);
}

void AfeManager::updateAfeAuxVoltage(std::string &projectName, std::string &payload){
  loadAfeInfo(projectName); 

  m_afeDatagram.deserialize(payload); 
  uint16_t voltage = m_afeDatagram.getVoltage(); 
  uint16_t index = m_afeDatagram.getIndex();
  //uint16_t voltage = m_afeDatagram.getAuxVoltage(index);
  
  m_afeInfo["aux" + std::to_string(index)] = std::to_string(voltage) + "mv";
  
  saveAfeInfo(projectName);
}

void AfeManager::updateAfeCellDevVoltage(std::string &projectName, std::string &payload){
  loadAfeInfo(projectName); 

  m_afeDatagram.deserialize(payload); 
  uint16_t voltage = m_afeDatagram.getVoltage(); 

  std::size_t dev_index = m_afeDatagram.getDevIndex(); 

  const uint16_t start = dev_index * 18;
  const uint16_t end   = start + 18;
  
  for (uint16_t cell = start; cell < end; ++cell){
    //uint16_t voltage = m_afeDatagram.getCellVoltage(cell); 
    m_afeInfo["cell" + std::to_string(cell)] = std::to_string(voltage) + "mv"; 
  }
  
  saveAfeInfo(projectName); 
}

void AfeManager::updateAfeAuxDevVoltage(std::string &projectName, std::string &payload){
  loadAfeInfo(projectName); 

  m_afeDatagram.deserialize(payload);
  uint16_t voltage = m_afeDatagram.getVoltage(); 

  std::size_t dev_index = m_afeDatagram.getDevIndex(); 

  const uint16_t start = dev_index * 9; 
  const uint16_t end = start + 9; 

  for (uint16_t aux = start; aux < end; ++aux){
    //uint16_t voltage = m_afeDatagram.getAuxVoltage(aux); 
    m_afeInfo["aux" + std::to_string(aux)] = std::to_string(voltage) + "mv"; 
  }

  saveAfeInfo(projectName); 
}

void AfeManager::updateAfeCellPackVoltage(std::string &projectName, std::string &payload){
  loadAfeInfo(projectName); 

  m_afeDatagram.deserialize(payload); 
  uint16_t voltage = m_afeDatagram.getVoltage(); 

  for (std::size_t dev_index = 0; dev_index < 3; ++dev_index){
    const uint16_t start = dev_index * 18;
    const uint16_t end   = start + 18;

    for (uint16_t cell = start; cell < end; ++cell){
      //uint16_t voltage = m_afeDatagram.getCellVoltage(cell);
      m_afeInfo["cell" + std::to_string(cell)] = std::to_string(voltage) + "mv";
    }
  }
  saveAfeInfo(projectName); 
}

void AfeManager::updateAfeAuxPackVoltage(std::string &projectName, std::string &payload){
  loadAfeInfo(projectName); 

  m_afeDatagram.deserialize(payload); 
  uint16_t voltage = m_afeDatagram.getVoltage(); 

  for (std::size_t dev_index = 0; dev_index < 3; ++dev_index){
    const uint16_t start = dev_index * 9;
    const uint16_t end   = start + 9;

    for (uint16_t aux = start; aux < end; ++aux){
      //uint16_t voltage = m_afeDatagram.getAuxVoltage(aux);
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
        uint8_t idx = static_cast<uint8_t>(std::stoi(index));
        uint16_t voltage = static_cast<uint16_t>(std::stoi(data)); 

        m_afeDatagram.setIndex(idx);
        m_afeDatagram.setVoltage(voltage);
        std::cout << "Data is: " << data << std::endl; 
        std::cout << "Voltage is: " << +m_afeDatagram.getVoltage() << std::endl;
        break;
      }
      case CommandCode::AFE_SET_AUX: {
        uint8_t idx = static_cast<uint8_t>(std::stoi(index));
        m_afeDatagram.setIndex(idx);
        m_afeDatagram.setVoltage(static_cast<uint16_t>(std::stoi(data)));
        break;
      }

      case CommandCode::AFE_SET_DEV_CELL: {
        m_afeDatagram.setDeviceIndex(std::stoul(index)); 
        m_afeDatagram.setVoltage(static_cast<uint16_t>(std::stoi(data)));
        break;
      }
      case CommandCode::AFE_SET_DEV_AUX: {
        m_afeDatagram.setDeviceIndex(std::stoul(index));
        m_afeDatagram.setVoltage(static_cast<uint16_t>(std::stoi(data)));
        break;
      }

      case CommandCode::AFE_SET_PACK_AUX: 
      case CommandCode::AFE_SET_PACK_CELL: {
        m_afeDatagram.setVoltage(static_cast<uint16_t>(std::stoi(data)));
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
        std::size_t idx = std::stoul(index);
        m_afeDatagram.setDeviceIndex(idx); 
        break;
      }
      case CommandCode::AFE_GET_DEV_AUX: {
        uint8_t idx = std::stoul(index);
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
