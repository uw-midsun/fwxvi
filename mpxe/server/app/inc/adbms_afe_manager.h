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
 * @details This class is responsible for transmitting serialized messages for reading cell/thermistor
 *          information. It shall support reading all cells/thermistors or individual ones
 *          The class shall cache current Afe data using a hash-map.
 */
class AfeManager {
 private:
  /** @brief Hash maps to store Afe information */
  using AfeObjectInfo = std::unordered_map<std::string, std::string>;
  std::unordered_map<std::string, AfeObjectInfo> m_afeInfo; /**< Hash map to store Afe data */

  Datagram::ADBMS_AFE m_afeDatagram; /**< Datagram object to serialize/deserialize payloads */

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
   * @brief   Update a single thermistor voltage for a project using the provided payload
   * @details Parses the payload to extract the voltage and index of the targeted thermistor input,
   *          then updates the corresponding thermistor field in the project info map
   * @param   projectName Name of the project to be updated
   * @param   payload Message data payload containing voltage and index
   */
  void updateAfeThermVoltage(std::string &projectName, std::string &payload);

  /**
   * @brief   Update all cell voltages for a specific AFE device using the provided payload
   * @details Calculates the voltage for a given device index, and updates voltages for
   *          all cells belonging to that device in the project info map
   * @param   projectName Name of the project to be updated
   * @param   payload Message data payload containing voltage and device index
   */
  void updateAfeCellDevVoltage(std::string &projectName, std::string &payload);

  /**
   * @brief   Update all thermistor voltages for a specific AFE device using the provided payload
   * @details Calculates the voltage for a given device index, and updates voltages for
   *          all thermistor inputs belonging to that device in the project info map
   * @param   projectName Name of the project to be updated
   * @param   payload Message data payload containing voltage and device index
   */
  void updateAfeThermDevVoltage(std::string &projectName, std::string &payload);

  /**
   * @brief   Update all cell voltages across the entire AFE pack using the provided payload
   * @details Iterates across all devices in the pack and updates each of their
   *          cell voltages with the value extracted from the payload
   * @param   projectName Name of the project to be updated
   * @param   payload Message data payload containing shared voltage value
   */
  void updateAfeCellPackVoltage(std::string &projectName, std::string &payload);

  /**
   * @brief   Update all thermistor voltages across the entire AFE pack using the provided payload
   * @details Iterates across all devices in the pack and updates each of their
   *          thermistor voltages with the value extracted from the payload
   * @param   projectName Name of the project to be updated
   * @param   payload Message data payload containing shared voltage value
   */
  void updateAfeThermPackVoltage(std::string &projectName, std::string &payload);

  /**
   * @brief   Update board's thermistor voltage for a specific device
   * @param   projectName Name of the project to be updated
   * @param   payload Message data payload containing shared voltage value
   */
  void updateAfeBoardThermVoltage(std::string &projectName, std::string &payload);

  /**
   * @brief   Update the discharge state of a specific AFE cell using the provided payload
   * @details Loads the target AFE project, extracts the discharge state of the given cell index
   *          from the deserialized payload, and updates the corresponding entry in the project info.
   *          Saves the updated discharge state to persistent storage.
   * @param   projectName Name of the project to be updated
   * @param   payload Serialized datagram payload containing the target cell index and discharge state
   */
  void updateAfeCellDischarge(std::string &projectName, std::string &payload);

  /**
   * @brief   Update the discharge state of the whole AFE pack using the provided payload
   * @details Loads the target AFE project, extracts the discharge state of the cell pack
   *          from the deserialized payload, and updates the corresponding entry in the project info
   *          Saves the updated discharge state to persistent storage
   * @param   projectName Name of the project to be updated
   * @param   payload Serialized datagram payload containting the discharge state
   */
  void updateAfeCellPackDischarge(std::string &projectName, std::string &payload);

  /**
   * @brief   Create a Afe Command object given a CommandCode, and if required specific data
   * @details This function shall support all ADBMS AFE CommandCodes
   * @param   commandCode Command code
   * @param   data Data payload to be transmitted
   * @return  Fully serialized data payload to be transmitted to the client
   */
  std::string createAfeCommand(CommandCode commandCode, std::string index, std::string data);
};

/** @} */
