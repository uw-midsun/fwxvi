/************************************************************************************************
 * @file   i2c_manager.cc
 *
 * @brief  Source file defining the I2CManager Class for the server
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>

/* Inter-component Headers */
#include "command_code.h"

/* Intra-component Headers */
#include "app.h"
#include "i2c_manager.h"

std::string I2CManager::stringifyPort(Datagram::12C::Port port){
  switch(port){
    case Datagram::I2C::Port::I2C1: return "I2C1";
    case Datagram::I2C::Port::I2C2: return "I2C2";
    default:return "UNKNOWN";
  }
}
std::string strinigifyCommandCode(CommandCode code){

}

void I2CManager::loadI2CConfig(std::string &projectName) {
    m_i2cInfo = serverJSONManager.getProjectValue<std::unordered_map<std::string, I2CData>>(projectName, I2C_KEY);
    
    m_deviceCache.clear();
    
    for(const auto& entry : m_i2cInfo){
      if (entry.first.find(DEVICE_KEY)==0){
        std::string deviceStr=entry.first.substr(strlen(DEVICE_KEY)+1);
        uint8_t deviceAddr=static_cast<uint8_t>(std::stoi(deviceStr,nullptr, 16));
        RegisterMap registers;
        for (const auto& transaction:entry.second){
          if(transaction.size()>=2){
            uint8_t regAddr=transaction[0];
            uint8_t value=transaction[1];
            registers[regAddr]=value;
          }
        }
        if(!registers.empty()){
          m_deviceCache[deviceAddr]=registers;
        }
      }
    }
  }

  void I2CManager::saveI2CConfig(std::string &projectName){
    std::unordered_map<std::string, I2CData> i2cConfigData;

    for (const auto& device:m_deviceCache){
      uint8_t deviceAddr=device.first;
      std::string deviceKey=std::string(DEVICE_KEY)+"_"+std::to_string(deviceAddr);
      I2CData transactions;
      for (const auto& reg:device.second){
        std::vector<uint8_t> transaction={reg.frst, reg.second};
        transactions.push_back(transaction);
      }
      if(!transaction.empty()){
        i2cConfigData[deviceKey]=transactions;
      }
    }
    serverJSONManager.setProjectValue(projectName, I2C_KEY, i2cConfigData);
    m_i2cInfo.clear();
  }

  std::string I2CManager::createWriteCommand(uint8_t deviceAddr, uint8_t regAddr, uint8_t value){
    try{
      m_i2cDatagra
    }
  }