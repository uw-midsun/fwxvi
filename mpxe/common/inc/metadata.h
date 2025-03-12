#pragma once

/************************************************************************************************
 * @file   metadata.h
 *
 * @brief  Header file defining the Metadata class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <string>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup MetadataDatagram
 * @brief    Shared Metadata Datagram class
 * @{
 */

namespace Datagram {

/**
 * @class   DatagramMetadata
 * @brief   Class for managing client Metadata and data transfer operations
 * @details This class provides an interface for configuring and managing client Metadata
 *          It supports serialization for data transfer
 */
class Metadata {
 public:
  /**
   * @brief   Metadata Datagram payload storage
   */
  struct Payload {
    std::string projectName;   /**< String project name */
    std::string projectStatus; /**< String project status */
    std::string hardwareModel; /**< String hardware model */
  };

  /**
   * @brief   Constructs a Metadata object with provided payload data
   * @param   data Reference to payload data
   */
  explicit Metadata(Payload &data);

  /**
   * @brief   Default constructor for Metadata object
   */
  Metadata() = default;

  /**
   * @brief   Serializes Metadata data with the Metadata command code for transmission
   * @return  Serialized string containing Metadata data
   */
  std::string serialize() const;

  /**
   * @brief   Deserializes Metadata data from payload string
   * @param   metadataPayload String containing serialized Metadata data
   */
  void deserialize(std::string &metadataPayload);

  /**
   * @brief   Sets the target project name
   * @param   projectName Updated project name
   */
  void setProjectName(const std::string &projectName);

  /**
   * @brief   Sets the target project status
   * @param   projectStatus Updated project status
   */
  void setProjectStatus(const std::string &projectStatus);

  /**
   * @brief   Sets the target hardware model
   * @param   hardwareModel Updated hardware model
   */
  void setHardwareModel(const std::string &hardwareModel);

  /**
   * @brief   Gets the target project name
   * @return  Target project name
   */
  std::string getProjectName() const;

  /**
   * @brief   Gets the target project status
   * @return  Target project status
   */
  std::string getProjectStatus() const;

  /**
   * @brief   Gets the target hardware model
   * @return  Target hardware model
   */
  std::string getHardwareModel() const;

 private:
  Payload m_metadata; /**< Private datagram payload */
};

}  // namespace Datagram

/** @} */
