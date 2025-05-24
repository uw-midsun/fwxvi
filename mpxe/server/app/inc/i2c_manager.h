#pragma once

/************************************************************************************************
 * @file   i2c_manager.h
 *
 * @brief  Header file defining the Server I2CManager class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>
#include <string.h>
#include <unordered_map>

/* Inter-component Headers */
#include "i2c_datagram.h"

#define I2C_KEY "i2c"
#define DEVICE_KEY "device"
#define REGISTER_KEY "register"
#define VALUE_KEY "value"


/* Intra-component Headers */

/**
 * @defgroup ServerI2CManager
 * @brief    I2CManager for the Server
 * @{
 */

/**
 * @class   ServerI2CManager
 * @brief   Class that manages receiving and transmitting I2C commands and JSON logging
 * @details This class is responsible for transmitting serialized messages for setting/retrieving RX/TX data buffers
 *          The class shall provide an interface to control I2C transactions
 */
class I2CManager {
 
 public:

 I2CManager()=default;
 using RegisterMap=std::unordered_map<uint8_t, uint8_t>;
 using DeviceMap=std::unordered_map<uint8_t, RegisterMap>;

 std::string createWriteCommand(uint8_t deviceAddr, uint8_t regAddr, uint8_t value);

 std::string createReadCommand(uint8_t deviceAddr, uint8_t regAddr);

 void updateRegister(uint8_t deviceAddr, uint8_t regAddr, uint8_t value);
 void updateAllRegisters(uint8_t deviceAddr, uint8_t regAddr, const std::string& payload);
 uint8_t readRegister(uint8_t deviceAddr, uint8_t regAddr);
 const DeviceMap& getDeviceCache() const { return m_deviceCache; }

 
  /**
   * @brief   Constructs a I2CManager object
   * @details Default constructor
   */


  I2CManager() = default;
private:
 DeviceMap m_deviceCache; //List of connected slaves
 using I2CData=std::vector<std::vector<uint8_t>>;
 //8-bit byte data packets
 std::unordered_map<std::string, I2CData> m_i2cInfo;
 Datagram::I2C m_i2cDatagram;
 std::string stringifyPort(Datagram::I2C::Port Port){
  switch (Port) {
    case Datagram::I2C::Port::I2C_PORT_1: return "I2C1";
    case Datagram::I2C::Port::I2C_PORT_2: return "I2C2";
    default: return "UNKNOWN";
  }
 }

 std::string strigidyCommandCode(CommandCode code){
  switch(code){
    case CommandCode::I2C_SET_DATA: return "I2C_SET_DATA";
    case CommandCode::I2C_GET_DATA: return "I2C_GET_DATA";
    case CommandCode::I2C_CONFIGURE: return "I2C_CONFIGURE";
    case CommandCode::I2C_RESET: return "I2C_RESET";
    default: return "UNKNOWN_COMMAND";
  }
 }
 void loadI2CConfig(std::string &projectName);
 void saveI2CConfig(std::string &projectName);

 
};

/** @} */
