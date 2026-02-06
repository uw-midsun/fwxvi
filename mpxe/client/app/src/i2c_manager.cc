/************************************************************************************************
 * @file   i2c_manager.cc
 *
 * @brief  Source file defining the I2CManager class for the client
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <iostream>
#include <vector>

/* Inter-component Headers */
extern "C" {
#include "i2c.h"
}

#include "command_code.h"

/* Intra-component Headers */
#include "i2c_manager.h"
#include "app.h"

    // set data
    // read data
    // x_86
    //i2c_set_rx_Data
    //i2c_get_rx_data
    //Clear buffer? --> after we clear after, dont need to clear 
    // What is a buffer? (an array), we send the data into it
    //

std::string I2CManager::I2C_set_data(std::string &payload) {
    m_I2CDatagram.deserialize(payload); //Use I2C datagram

    auto port = static_cast<I2CPort>(m_I2CDatagram.getI2CPort());
    const uint8_t *buffer = m_I2CDatagram.getBuffer();
    size_t length = m_I2CDatagram.getBufferLength();

    i2c_set_rx_data(port, buffer, length);
    m_I2CDatagram.clearBuffer();

    return m_I2CDatagram.serialize(CommandCode::I2C_SET_DATA);
}

std::string I2CManager::I2C_get_data(std::string &payload) {
    m_I2CDatagram.deserialize(payload); //Use I2C datagram

    auto port = static_cast<I2CPort>(m_I2CDatagram.getI2CPort());
    uint16_t length = m_I2CDatagram.getBufferLength();
    uint8_t temp_buffer[length];

    i2c_get_tx_data(port, temp_buffer, length);
    m_I2CDatagram.setBuffer(temp_buffer, length);

    return m_I2CDatagram.serialize(CommandCode::I2C_GET_DATA);
}