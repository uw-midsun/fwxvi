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
#include "app.h"
#include "i2c_manager.h"

std::string I2CManager::writeI2CData(std::string &payload) {
  m_I2CDatagram.deserialize(payload);

  I2CPort port = static_cast<I2CPort>(m_I2CDatagram.getI2CPort());
  const uint8_t *buffer = m_I2CDatagram.getBuffer();
  size_t length = m_I2CDatagram.getBufferLength();

  i2c_set_rx_data(port, buffer, length);
  m_I2CDatagram.clearBuffer();

  return m_I2CDatagram.serialize(CommandCode::I2C_WRITE_DATA);
}

std::string I2CManager::readI2CData(std::string &payload) {
  m_I2CDatagram.deserialize(payload);

  I2CPort port = static_cast<I2CPort>(m_I2CDatagram.getI2CPort());
  size_t length = m_I2CDatagram.getBufferLength();
  if (length == 0U) {
    length = i2c_get_tx_num_bytes(port);
  }

  std::vector<uint8_t> temp_buffer(length);

  i2c_get_tx_data(port, temp_buffer.data(), length);
  m_I2CDatagram.setBuffer(temp_buffer.data(), length);

  return m_I2CDatagram.serialize(CommandCode::I2C_READ_DATA);
}

std::string I2CManager::clearI2CBuffers(std::string &payload) {
  m_I2CDatagram.deserialize(payload);

  I2CPort port = static_cast<I2CPort>(m_I2CDatagram.getI2CPort());

  i2c_clear_buffers(port);
  m_I2CDatagram.clearBuffer();

  return m_I2CDatagram.serialize(CommandCode::I2C_CLEAR_BUFFER);
}
