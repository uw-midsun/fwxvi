#pragma once

/************************************************************************************************
 * @file   adbms_afe_manager.h
 *
 * @brief  Header file defining the Server AfeManager class
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

    /**
     * @brief   Update a single cell voltage for a project using the provided payload
     * @details Parses the payload to extract the voltage and index of the targeted cell,
     *          then updates the corresponding cell field in the project info map
     * @param   projectName Name of the project to be updated
     * @param   payload Message data payload containing voltage and index
     */
    void updateAfeCellVoltage(std::string &projectName, std::string &payload);

    /**
     * @brief   Update a single auxiliary voltage for a project using the provided payload
     * @details Parses the payload to extract the voltage and index of the targeted auxiliary input,
     *          then updates the corresponding aux field in the project info map
     * @param   projectName Name of the project to be updated
     * @param   payload Message data payload containing voltage and index
     */
    void updateAfeAuxVoltage(std::string &projectName, std::string &payload);

    /**
     * @brief   Update all cell voltages for a specific AFE device using the provided payload
     * @details Calculates the voltage for a given device index, and updates voltages for
     *          all cells belonging to that device in the project info map
     * @param   projectName Name of the project to be updated
     * @param   payload Message data payload containing voltage and device index
     */
    void updateAfeCellDevVoltage(std::string &projectName, std::string &payload);

    /**
     * @brief   Update all auxiliary voltages for a specific AFE device using the provided payload
     * @details Calculates the voltage for a given device index, and updates voltages for
     *          all auxiliary inputs belonging to that device in the project info map
     * @param   projectName Name of the project to be updated
     * @param   payload Message data payload containing voltage and device index
     */
    void updateAfeAuxDevVoltage(std::string &projectName, std::string &payload);

    /**
     * @brief   Update all cell voltages across the entire AFE pack using the provided payload
     * @details Iterates across all devices in the pack and updates each of their
     *          cell voltages with the value extracted from the payload
     * @param   projectName Name of the project to be updated
     * @param   payload Message data payload containing shared voltage value
     */
    void updateAfeCellPackVoltage(std::string &projectName, std::string &payload);

    /**
     * @brief   Update all auxiliary voltages across the entire AFE pack using the provided payload
     * @details Iterates across all devices in the pack and updates each of their
     *          auxiliary voltages with the value extracted from the payload
     * @param   projectName Name of the project to be updated
     * @param   payload Message data payload containing shared voltage value
     */
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

/** @} */
