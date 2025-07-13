/* Standard library Headers */
#include <cstdint>

/* Inter-component Headers */
#include "command_code.h"

/* Intra-component Headers */
#include "app.h"
#include "server/app/inc/adbms_afe_manager.h"


#define AFE_KEY "afe"

void AfeManager::loadAfeInfo(std::string &projectName){
  m_afeInfo = serverJSONManager.getProjectValue<std::unordered_map<std::string, std::string>>(projectName, AFE_KEY);
}

void AfeManager::saveAfeInfo(std::string &projectName){
  serverJSONManager.setProjectValue(projectName, AFE_KEY, m_afeInfo);

  /* Upon save, clear the memory */
  m_afeInfo.clear(); 
}

void AfeManager::updateCellVoltage(std::string &projectName, std::string &payload){
  loadAfeInfo(projectName); 

  m_afeDatagram.deserialize(payload); 
  uint16_t index = m_afeDatagram.getIndex();
  uint8_t voltage = m_afeDatagram.getCellVoltage(index);
  
  m_afeInfo["Cell" + std::to_string(index)] = std::to_string(voltage) + "mv";
  
  saveAfeInfo(projectName);
}

void AfeManager::updateAfeAuxVoltage(std::string &projectName, std::string &payload){
  loadAfeInfo(projectName); 

  m_afeDatagram.deserialize(payload); 
  uint16_t index = m_afeDatagram.getIndex();
  uint8_t voltage = m_afeDatagram.getAuxVoltage(index);
  
  m_afeInfo["Aux" + std::to_string(idx)] = std::to_string(voltage) + "mv";
  
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
    m_afeInfo["Cell" + std::to_string(cell)] = std::to_string(voltage) + "mv"; 
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
    m_afeInfo["Aux" + std::to_string(aux)] = std::to_string(voltage) + "mv"; 
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
      m_afeInfo["Cell" + std::to_string(cell)] = std::to_string(voltage) + "mv";
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
      uint16_t voltage = m_afeDatagram.getCellVoltage(aux);
      m_afeInfo["Aux" + std::to_string(aux)] = std::to_string(voltage) + "mv";
    }
  }

  saveAfeInfo(projectName); 
}

std::string AfeManager::createAfeCommand(CommandCode commandCode, std::string channel, std::string data){
  try {
    switch(commandCode){
      /* Setters */
      case CommandCode::AFE_SET_CELL:
      case CommandCode::AFE_SET_AUX: {
        if (channel.rfind("Cell", 0) == 0){
          std::size_t idx = std::stoul(channel.substr(4));
          m_afeDatagram.setIndex(idx); 
        }
        else if (channel.rfind("Aux", 0) == 0){
          std::size_t idx = std::stoul(channel.substr(4)); 
          m_afeDatagram.setIndex(idx);
        }
        else{
          throw std::runtime_error("Invalid channel specifier. Good examples: 'Cell0' 'Cell1' 'Aux1' 'Aux0'");
        }
        m_afeDatagram.setVoltage(static_cast<uint8_t>(std::stoul(data)));
        break;
      }
      case CommandCode::AFE_SET_DEV_CELL:
      case CommandCode::AFE_SET_DEV_AUX: {
        if (channel.rfind("Cell", 0) == 0){
          std::size_t idx = std::stoul(channel.substr(4));
          m_afeDatagram.setDeviceIndex(idx); 
        }
        else if (channel.rfind("Aux", 0) == 0){
          std::size_t idx = std::stoul(channel.substr(4)); 
          m_afeDatagram.setDeviceIndex(idx);
        }
        else{
          throw std::runtime_error("Invalid channel specifier. Good examples: 'Cell0' 'Cell1' 'Aux1' 'Aux0'. "
                                    "Note for this command, the number represents the device index.");
        }
        m_afeDatagram.setVoltage(static_cast<uint8_t>(std::stoul(data)));
        break;
      }
      case CommandCode::AFE_SET_PACK_AUX:
      case CommandCode::AFE_SET_PACK_CELL: {
        m_afeDatagram.setVoltage(static_cast<uint8_t>(std::stoul(data)));
        break;
      }

      /* Getters */
      case CommandCode::AFE_GET_CELL:
      case CommandCode::AFE_GET_AUX: {
        if (channel.rfind("Cell", 0) == 0){
          std::size_t idx = std::stoul(channel.substr(4));
          m_afeDatagram.setIndex(idx); 
        }
        else if (channel.rfind("Aux", 0) == 0){
          std::size_t idx = std::stoul(channel.substr(4)); 
          m_afeDatagram.setIndex(idx);
        }
        else{
          throw std::runtime_error("Invalid channel specifier. Good examples: 'Cell0' 'Cell1' 'Aux1' 'Aux0'");
        }
        break;
      }
      case CommandCode::AFE_GET_DEV_CELL:
      case CommandCode::AFE_GET_DEV_AUX: {
        if (channel.rfind("Cell", 0) == 0){
          std::size_t idx = std::stoul(channel.substr(4));
          m_afeDatagram.setDeviceIndex(idx); 
        }
        else if (channel.rfind("Aux", 0) == 0){
          std::size_t idx = std::stoul(channel.substr(4)); 
          m_afeDatagram.setDeviceIndex(idx);
        }
        else{
          throw std::runtime_error("Invalid channel specifier. Good examples: 'Cell0' 'Cell1' 'Aux1' 'Aux0'. "
                                    "Note for this command, the number represents the device index.");
        }
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
  }
  return m_afeDatagram.serialize(commandCode);
  catch(const std::exception& e){
    std::cerr << "Afe Manager errror: " << e.what() << std::endl;
  }
  return "";
}
