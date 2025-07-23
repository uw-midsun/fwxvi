#pragma once

/************************************************************************************************
 * @file   adc_manager.h
 *
 * @brief  Header file defining the Client AdcManager class
 *
 * @date   2025-04-24
 * @author Shahzaib Siddiqui
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>
#include <string.h>

#include <unordered_map>

/* Inter-component Headers */
#include "adc_datagram.h"

/* Intra-component Headers */

/**
 * @defgroup ClientAdcManager
 * @brief    AdcManager for the Client
 * @{
 */

/**
 * @class   AdcManager
 * @brief   Class that manages receiving and transmitting Adc data readings
 * @details This class is responsible for handling and transmitting serialized messages for reading pin modes,
 *          alternate functions and states. It shall support reading all pins or individual pins
 *          The class shall perform procedures as soon as commands are queued by the client
 */
class AdcManager {
 private:
  Datagram::Adc m_adcDatagram; /**< Datagram class to serialize/deserialize commands */

 public:
  /**
   * @brief   Constructs a AdcManager object
   * @details Default constructor
   */
  AdcManager() = default;

  /**
   * @brief   Sets the raw ADC value given the data payload
   * @details This function shall be called upon receiving a pin-specific payload
   * @param   payload Message data payload to be parsed
   */
  void setAdcRaw(std::string &payload);

  /**
   * @brief   Sets all raw ADC values given the data payload
   * @details This function shall be called upon receiving a payload containing all pin data
   * @param   payload Message data payload to be parsed
   */
  void setAdcAllRaw(std::string &payload);

  /**
   * @brief   Processes a get request for a specific raw ADC channel reading
   * @details This function shall be called upen receiving a pin-specific payload
   * @param   payload Message data payload to be parsed
   * @return  Fully serialized data payload to be transmitted in response to the server
   */
  std::string processReadAdcRaw(std::string &payload);

  /**
   * @brief   Processes a get request for all raw ADC channel readings
   * @details This function shall be called upon receiving a payload containing all pin data
   * @return  Fully serialized data payload to be transmitted in response to the server
   */
  std::string processReadAdcAllRaw();

  /**
   * @brief   Processes a get request for a specific converted ADC channel reading
   * @param   payload Message data payload to be parsed
   * @details This function shall be called upen receiving a pin-specific payload
   * @return  Fully serialized data payload to be transmitted in response to the server
   */
  std::string processReadAdcConverted(std::string &payload);

  /**
   * @brief   Processes a get request for all converted ADC channel readings
   * @details This function shall be called upon receiving a payload containing all pin data
   * @return  Fully serialized data payload to be transmitted in response to the server
   */
  std::string processReadAdcAllConverted();
};

/** @} */
