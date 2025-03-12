#pragma once

/************************************************************************************************
 * @file   gpio_manager.h
 *
 * @brief  Header file defining the Client GpioManager class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>
#include <string.h>

#include <unordered_map>

/* Inter-component Headers */
#include "gpio_datagram.h"

/* Intra-component Headers */

/**
 * @defgroup ClientGpioManager
 * @brief    GpioManager for the Client
 * @{
 */

/**
 * @class   GpioManager
 * @brief   Class that manages receiving and transmitting Gpio commands and JSON logging
 * @details This class is responsible for handling and transmitting serialized messages for reading pin modes,
 *          alternate functions and states. It shall support reading all pins or individual pins
 *          The class shall perform procedures as soon as commands are queued by the client
 */
class GpioManager {
 private:
  Datagram::Gpio m_gpioDatagram; /**< Datagram class to serialize/deserialize commands */

 public:
  /**
   * @brief   Constructs a GpioManager object
   * @details Default constructor
   */
  GpioManager() = default;

  /**
   * @brief   Sets a Gpio Pin state given the data payload
   * @details This function shall be called upon receiving a pin-specific payload
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload to be parsed
   */
  void setGpioPinState(std::string &payload);

  /**
   * @brief   Sets all Gpio Pin state given the data payload
   * @details This function shall be called upon receiving a payload containing all pin data
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload to be parsed
   */
  void setGpioAllStates(std::string &payload);

  /**
   * @brief   Process a get Gpio Pin state command given the data payload
   * @details This function shall be called upon receiving a pin-specific payload
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload to be parsed
   * @return  Fully serialized data payload to be transmitted in response to the server
   */
  std::string processGpioPinState(std::string &payload);

  /**
   * @brief   Process a get all Gpio Pin states command
   * @details This function shall be called upon receiving a payload containing all pin data
   *          This function is not responsible for handling update errors
   * @return  Fully serialized data payload to be transmitted in response to the server
   */
  std::string processGpioAllStates();

  /**
   * @brief   Process a get Gpio Pin mode command given the data payload
   * @details This function shall be called upon receiving a pin-specific payload
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload to be parsed
   * @return  Fully serialized data payload to be transmitted in response to the server
   */
  std::string processGpioPinMode(std::string &payload);

  /**
   * @brief   Process a get all Gpio Pin modes command
   * @details This function shall be called upon receiving a payload containing all pin data
   *          This function is not responsible for handling update errors
   * @return  Fully serialized data payload to be transmitted in response to the server
   */
  std::string processGpioAllModes();

  /**
   * @brief   Process a get Gpio Pin alternate function command given the data payload
   * @details This function shall be called upon receiving a pin-specific payload
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload to be parsed
   * @return  Fully serialized data payload to be transmitted in response to the server
   */
  std::string processGpioPinAltFunction(std::string &payload);

  /**
   * @brief   Process a get all Gpio Pin alternate functions command
   * @details This function shall be called upon receiving a payload containing all pin data
   *          This function is not responsible for handling update errors
   * @return  Fully serialized data payload to be transmitted in response to the server
   */
  std::string processGpioAllAltFunctions();
};

/** @} */
