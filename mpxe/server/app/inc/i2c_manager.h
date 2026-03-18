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

/* Inter-component Headers */
#include "i2c_datagram.h"

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
 private:

  /**
   * @brief   Save cached ADC readings for a project to JSON storage
   * @param   projectName Name of the project whose ADC info should be saved
   */
  void loadI2CInfo(std::string &projectName);

  /**
   * @brief   Save cached ADC readings for a project to JSON storage
   * @param   projectName Name of the project whose ADC info should be saved
   */
  void saveI2CInfo(std::string &projectName);

  /**
   * @brief   Save cached ADC readings for a project to JSON storage
   * @param   projectName Name of the project whose ADC info should be saved
   */
  void loadAdcInfo(std::string &projectName);

  //P
 public:

  /**
   * @brief   Constructs a I2CManager object
   * @details Default constructor
   */

   void updateSetData(std::string &projectName, std::string &payload);

   /**
   * @brief   Constructs a I2CManager object
   * @details Default constructor
   */

   void updateReceiveData(std::string &projectName, std::string &payload);



  /**
   * @brief   Constructs a I2CManager object
   * @details Default constructor
   */
  I2CManager() = default;
};

/** @} */
