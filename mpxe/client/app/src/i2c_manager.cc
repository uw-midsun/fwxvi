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

void I2CManager::I2C_Transmit(std::string &payload) {
    m_I2CDatagram.deserialize(payload); //Use I2C datagram
    // set data
    // read data
    // x_86
    //i2c_set_rx_Data
    //i2c_get_rx_data
    //Clear buffer? --> after we clear after, dont need to clear 
    // What is a buffer? (an array), we send the data into it
    //

    auto port = static_cast<I2CPort>(m_I2CDatagram.getI2CPort());
    const uint8_t *buffer = m_I2CDatagram.getBuffer();
    size_t length = m_I2CDatagram.getBufferLength();

    if (buffer == nullptr || length == 0 || length > I2C_MAX_NUM_DATA) {
        return;
    }

    constexpr I2CAddress DEVICE_ADDR = 0x0067;  // doesnt implement it in the datagram

    StatusCode status = i2c_write(
        port,
        DEVICE_ADDR,
        const_cast<uint8_t *>(buffer),
        length
    );

    if (status != STATUS_CODE_OK) {
        // log / assert / error handling
    }
}

