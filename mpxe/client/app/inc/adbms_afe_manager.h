#pragma once

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
 * @details This class is responsible for handling and transmitting serialized messages for reading pin modes,
 *          alternate functions and states. It shall support reading all pins or individual pins
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
  AfeManager(); 

  void setAfeCell(std::string &payload); 
  void setAfeAux(std::string &payload);

  void setAfeDevCell(std::string &payload);
  void setAfeDevAux(std::string &payload);

  void setAfePackCell(std::string &payload);
  void setAfePackAux(std::string &payload); 


  std::string processAfeCell(std::string &payload); 
  std::string processAfeAux(std::string &payload);

  std::string processAfeDevCell(std::string &payload);
  std::string processAfeDevAux(std::string &payload);

  std::string processAfePackCell();
  std::string processAfePackAux(); 
};

/** @} */