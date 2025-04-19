#pragma once

/************************************************************************************************
 * @file   gpio_manager.h
 *
 * @brief  Header file defining the Server GpioManager class
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
 * @defgroup ServerGpioManager
 * @brief    GpioManager for the Server
 * @{
 */

/**
 * @class   GpioManager
 * @brief   Class that manages receiving and transmitting Gpio commands and JSON logging
 * @details This class is responsible for transmitting serialized messages for reading pin modes,
 *          alternate functions and states. It shall support reading all pins or individual pins
 *          The class shall cache current Gpio data using a hash-map
 */
class GpioManager {
 private:
  /** @brief  Hash-map to store Pin specific data such as state, mode, or alternate function */
  using PinInfo = std::unordered_map<std::string, std::string>;

  std::unordered_map<std::string, PinInfo> m_gpioInfo; /**< Hash-map to cache all Gpio data */
  Datagram::Gpio m_gpioDatagram;                       /**< Datagram class to serialize/deserialize commands */

  /**
   * @brief   Matches a string based on the received Gpio mode
   * @param   mode Received mode that shall be stringified
   * @return  String that matches the input mode
   */
  std::string stringifyPinMode(Datagram::Gpio::Mode mode);

  /**
   * @brief   Matches a string based on the received Gpio mode
   * @param   altFunction Received alternate function that shall be stringified
   * @return  String that matches the input altFunction
   */
  std::string stringifyPinAltFunction(Datagram::Gpio::AltFunction altFunction);

  /**
   * @brief   Loads the Hash-map cache with a projects Gpio data
   * @param   projectName Selector for which project shall be loaded into the cache
   */
  void loadGpioInfo(std::string &projectName);

  /**
   * @brief   Saves the Hash-map cache to a projects Gpio data
   * @param   projectName Selector for which project shall be updated with the current cache
   */
  void saveGpioInfo(std::string &projectName);

 public:
  /**
   * @brief   Constructs a GpioManager object
   * @details Default constructor
   */
  GpioManager() = default;

  /**
   * @brief   Update a Gpio Pin state for a provided project given the data payload
   * @details This function shall be called upon receiving a pin-specific payload
   *          Determining if this function is applicable is the responsibility of the terminal/UI
   * @param   projectName Name of the project to be updated
   * @param   payload Message data payload to be parsed
   */
  void updateGpioPinState(std::string &projectName, std::string &payload);

  /**
   * @brief   Update all Gpio Pin states for a provided project given the data payload
   * @details This function shall be called upon receiving a payload containing all pin data
   *          Determining if this function is applicable is the responsibility of the terminal/UI
   * @param   projectName Name of the project to be updated
   * @param   payload Message data payload to be parsed
   */
  void updateGpioAllStates(std::string &projectName, std::string &payload);

  /**
   * @brief   Update a Gpio Pin mode for a provided project given the data payload
   * @details This function shall be called upon receiving a payload containing all pin data
   *          Determining if this function is applicable is the responsibility of the terminal/UI
   * @param   projectName Name of the project to be updated
   * @param   payload Message data payload to be parsed
   */
  void updateGpioPinMode(std::string &projectName, std::string &payload);

  /**
   * @brief   Update all Gpio Pin modes for a provided project given the data payload
   * @details This function shall be called upon receiving a payload containing all pin data
   *          Determining if this function is applicable is the responsibility of the terminal/UI
   * @param   projectName Name of the project to be updated
   * @param   payload Message data payload to be parsed
   */
  void updateGpioAllModes(std::string &projectName, std::string &payload);

  /**
   * @brief   Update a Gpio Pin alternate function for a provided project given the data payload
   * @details This function shall be called upon receiving a payload containing all pin data
   *          Determining if this function is applicable is the responsibility of the terminal/UI
   * @param   projectName Name of the project to be updated
   * @param   payload Message data payload to be parsed
   */
  void updateGpioPinAltFunction(std::string &projectName, std::string &payload);

  /**
   * @brief   Update all Gpio Pin alternate functions for a provided project given the data payload
   * @details This function shall be called upon receiving a payload containing all pin data
   *          Determining if this function is applicable is the responsibility of the terminal/UI
   * @param   projectName Name of the project to be updated
   * @param   payload Message data payload to be parsed
   */
  void updateGpioAllAltFunctions(std::string &projectName, std::string &payload);

  /**
   * @brief   Create a Gpio command given a CommandCode and if required, a specific Port/Pin map and data
   * @details This function shall support all Gpio CommandCodes
   *          If the command is requesting data from all pins, no gpioPortPin and data is required
   * @param   commandCode Command reference to be transmitted to the client
   * @param   gpioPortPin Request reference to the port and pin. Must be in this form: 'A9' or 'B12' or 'C14' etc.
   * @param   data Data payload to be transmitted. This parameter currently only supports SET_STATE commands and can be set to 'LOW' or 'HIGH'
   * @return  Fully serialized data payload to be transmitted to the client
   */
  std::string createGpioCommand(CommandCode commandCode, std::string &gpioPortPin, std::string data);
};

/** @} */
