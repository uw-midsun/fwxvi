#pragma once

/************************************************************************************************
 * @file   adbms_afe_manager.cc
 *
 * @brief  Header file defining the AfeManager class for the client
 *
 * @date   2025-07-22
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>
#include <string.h>
#include <unordered_map>

/* Inter-component Headers */
#include "adbms_afe_datagram.h"

/* Intra-component Headers */

/**
 * @defgroup ClientAfeManager
 * @brief    AfeManager for the Client
 * @{
 */

 /**
 * @class   AfeManager
 * @brief   Class that manages receiving and transmitting Afe commands and JSON logging
 * @details This class is responsible for handling and transmitting serialized messages for reading voltages,
 *          and setting voltages. It shall support setting/reading all cells/aux or individual cells/aux
 *          The class shall perform procedures as soon as commands are queued by the client
 */

class AfeManager {
 private:
  Datagram::ADBMS_AFE m_afeDatagram; /**< Datagram class to serialize/deserialize commands */

 public:
  /**
   * @brief   Construct a AfeManager object
   * @details Custom constructor
   */
  AfeManager() = default; 

  /**
   * @brief   Sets a specific AFE cell voltage given the data payload
   * @details This function shall be called upon receiving a cell-specific payload
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload to be parsed
  */
  void setAfeCell(std::string &payload);

  /**
   * @brief   Sets a specific AFE auxiliary voltage given the data payload
   * @details This function shall be called upon receiving an aux-specific payload
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload to be parsed
   */
  void setAfeAux(std::string &payload);

  /**
   * @brief   Sets all AFE cell voltages for a single device given the data payload
   * @details This function shall be called upon receiving a device-specific payload
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload to be parsed
   */
  void setAfeDevCell(std::string &payload);

  /**
   * @brief   Sets all AFE auxiliary voltages for a single device given the data payload
   * @details This function shall be called upon receiving a device-specific payload
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload to be parsed
   */
  void setAfeDevAux(std::string &payload);

  /**
   * @brief   Sets all AFE cell voltages across the entire pack given the data payload
   * @details This function shall be called upon receiving a pack-wide payload
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload to be parsed
   */
  void setAfePackCell(std::string &payload);

  /**
   * @brief   Sets all AFE auxiliary voltages across the entire pack given the data payload
   * @details This function shall be called upon receiving a pack-wide payload
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload to be parsed
   */
  void setAfePackAux(std::string &payload);

  /**
   * @brief   Sets AFE discharge cells for individual cells
   * @details This function shall be called upon receiving a single-cell payload
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload to be parsed
   */
  void setCellDischarge(std::string &payload);

  /**
   * @brief   Process a get AFE cell voltage command given the data payload
   * @details This function shall be called upon receiving a cell-specific payload
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload to be parsed
   * @return  Fully serialized data payload to be transmitted in response to the server
   */
  std::string processAfeCell(std::string &payload);

  /**
   * @brief   Process a get AFE auxiliary voltage command given the data payload
   * @details This function shall be called upon receiving an aux-specific payload
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload to be parsed
   * @return  Fully serialized data payload to be transmitted in response to the server
   */
  std::string processAfeAux(std::string &payload);

  /**
   * @brief   Process a get AFE device-level cell voltage command given the data payload
   * @details This function shall be called upon receiving a device-specific payload
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload to be parsed
   * @return  Fully serialized data payload to be transmitted in response to the server
   */
  std::string processAfeDevCell(std::string &payload);

  /**
   * @brief   Process a get AFE device-level auxiliary voltage command given the data payload
   * @details This function shall be called upon receiving a device-specific payload
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload to be parsed
   * @return  Fully serialized data payload to be transmitted in response to the server
   */
  std::string processAfeDevAux(std::string &payload);

  /**
   * @brief   Process a get all AFE cell voltages across the entire pack
   * @details This function shall be called upon receiving a pack-wide request
   *          This function is not responsible for handling update errors
   * @return  Fully serialized data payload to be transmitted in response to the server
   */
  std::string processAfePackCell();

  /**
   * @brief   Process a get all AFE auxiliary voltages across the entire pack
   * @details This function shall be called upon receiving a pack-wide request
   *          This function is not responsible for handling update errors
   * @return  Fully serialized data payload to be transmitted in response to the server
   */
  std::string processAfePackAux();

  /**
   * @brief   Process a set or toggle cell discharge command from the server.
   * @details This function interprets a payload containing discharge instructions for a specific cell
   *          and updates the discharge bitset accordingly in the AFE configuration structure
   *          This function is not responsible for handling update errors
   * @param   payload Message data payload containing the target cell index and discharge state
   * @return  Serialized response payload to be sent back to the server (may be empty on success)
   */
  std::string processCellDischarge(std::string &payload);

};

/** @} */