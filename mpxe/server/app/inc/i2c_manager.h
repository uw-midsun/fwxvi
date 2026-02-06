#pragma once
/************************************************************************************************
 * @file   i2c_manager.h
 *
 * @brief  Header file defining the Server I2CManager class
 *
 * @date   2026-02-05
 * @author Jun Li
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>
#include <string.h>

#include <unordered_map>
#include <vector>

/* Inter-component Headers */
#include "i2c_datagram.h"

/* Intra-component Headers */

/**
 * @defgroup ServerI2CManager
 * @brief    I2CManager for the Server
 * @{
 */

/**
 * @class   I2CManager
 * @brief   Class that manages receiving and transmitting I2C commands and JSON logging
 * @details This class is responsible for transmitting serialized messages for reading/writing I2C data buffers.
 *          It shall support RX and TX data operations for I2C communication.
 *          The class shall cache current I2C data using a hash-map
 */
class I2CManager {
 private:
  /** @brief  Hash-map to store I2C port specific data such as RX/TX buffers */
  using PortInfo = std::unordered_map<std::string, std::vector<uint8_t>>;

  std::unordered_map<std::string, PortInfo> m_i2cInfo; /**< Hash-map to cache all I2C data */
  Datagram::I2C m_I2CDatagram;                         /**< Datagram class to serialize/deserialize commands */

  /**
   * @brief   Loads the Hash-map cache with a projects I2C data
   * @param   projectName Selector for which project shall be loaded into the cache
   */
  void loadI2CInfo(std::string &projectName);

  /**
   * @brief   Saves the Hash-map cache to a projects I2C data
   * @param   projectName Selector for which project shall be updated with the current cache
   */
  void saveI2CInfo(std::string &projectName);

 public:
  /**
   * @brief   Constructs an I2CManager object
   * @details Default constructor
   */
  I2CManager() = default;

  /**
   * @brief   Update I2C RX data for a provided project given the data payload
   * @details This function shall be called upon receiving an I2C set data payload
   *          Determining if this function is applicable is the responsibility of the terminal/UI
   * @param   projectName Name of the project to be updated
   * @param   payload Message data payload to be parsed
   */
  void updateI2CSetData(std::string &projectName, std::string &payload);

  /**
   * @brief   Update I2C TX data for a provided project given the data payload
   * @details This function shall be called upon receiving an I2C get data payload
   *          Determining if this function is applicable is the responsibility of the terminal/UI
   * @param   projectName Name of the project to be updated
   * @param   payload Message data payload to be parsed
   */
  void updateI2CGetData(std::string &projectName, std::string &payload);

  /**
   * @brief   Create an I2C command given a CommandCode and if required, specific port and data
   * @details This function shall support all I2C CommandCodes
   * @param   commandCode Command reference to be transmitted to the client
   * @param   i2cPort I2C port identifier (e.g., "I2C1", "I2C2", "I2C3")
   * @param   data Data payload to be transmitted
   * @return  Fully serialized data payload to be transmitted to the client
   */
  std::string createI2CCommand(CommandCode commandCode, std::string &i2cPort, std::vector<uint8_t> &data);
};

/** @} */