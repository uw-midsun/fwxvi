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

void I2CManager::updateSetData(std::string &projectName, std::string &payload){
    m_I2CDatagram.deserialize(payload);
    


}


