#pragma once



/* Standard library Headers */
#include <stdint.h>
#include <string.h>

#include <unordered_map>

/* Inter-component Headers */
#include "adbms_afe_datagram.h"

/* Intra-component Headers */


/**
 * @defgroup ServerAfeManager
 * @brief    AfeManager for the server
 * @{
 */

 /**
  * @class   AfeManager
  * @brief   Class that manages receiving and transmitting Afe commands and JSON logging
  * @details This class is... (Write later)
  */
 class AfeManager{
  private:
   std::unordered_map<std::string, std::string> m_afeInfo;
   Datagram::ADBMS_AFE m_afeDatagram;

   /**
    * @brief  Loads the Hash-map cache with a projects Afe data
    * @param  projectName Selector for which project shall be loaded into the cache
    */
    void loadAfeInfo(std::string &projectName); 

    /**
     * @brief Saves the Hash-map cache to a projects Afe data
     * @param projectName Selector for which project shall be loaded into the cache 
     */
    void saveAfeInfo(std::string &projectName);
  public:
    /**
     * @brief   Construct a new Afe Manager object
     * @details Default constructor 
     */
    AfeManager() = default; 

    void updateAfeCellVoltage(std::string &projectName, std::string &payload);
    void updateAfeAuxVoltage(std::string &projectName, std::string &payload); 

    void updateAfeCellDevVoltage(std::string &projectName, std::string &payload); 
    void updateAfeAuxDevVoltage(std::string &projectName, std::string &payload); 

    void updateAfeCellPackVoltage(std::string &projectName, std::string &payload);
    void updateAfeAuxPackVoltage(std::string &projectName, std::string &payload); 

    /**
     * @brief   Create a Afe Command object given a CommandCode, and if required specific data
     * @details This function shall support all ADBMS AFE CommandCodes
     * @param   commandCode Command code
     * @param   data Data payload to be transmitted 
     * @param   channel Optional (Eg: "Cell0", "Cell1", "Aux0", "Aux1")
     * @return  Fully serialized data payload to be transmitted to the client
     */
    std::string createAfeCommand(CommandCode commandCode, std::string index, std::string data);
};