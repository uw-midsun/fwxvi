#pragma once

/************************************************************************************************
 * @file   json_manager.h
 *
 * @brief  Header file defining the JSONManager class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

/* Inter-component Headers */
#include <nlohmann/json.hpp>

/* Intra-component Headers */

/**
 * @defgroup JSONManager
 * @brief    Shared JSON Manager class
 * @{
 */

#define PROJECT_VERSION "1.0.0"

/**
 * @class   JSONManager
 * @brief   Class for managing JSON Files
 * @details This class supports creating new JSON files, writing/reading to and from JSON files
 *          Default JSON generation and saving/loading data from provided paths
 */
class JSONManager {
 private:
  static constexpr const char *DEFAULT_JSON_PATH = "./Simulation_JSON/"; /**< Default JSON folder path */
  std::filesystem::path m_projectBasePath;                               /**<  Temporary variable to store project file path */

  /**
   * @brief   Creates a default project JSON
   * @details The project shall be created within the DEFAULT_JSON_PATH folder
   * @param   projectName Name of the project being created
   */
  void createDefaultProjectJSON(const std::string &projectName);

  /**
   * @brief   Get the file path of a specified project
   * @param   projectName Name of the project being retrieved
   * @return  File path relative to the root directory
   */
  std::filesystem::path getProjectFilePath(const std::string &projectName);

  /**
   * @brief   Loads the JSON of a specified project
   * @param   projectName Name of the project being retrieved
   * @return  JSON object of the selected project
   */
  nlohmann::json loadProjectJSON(const std::string &projectName);

  /**
   * @brief   Saves JSON data to a specified project
   * @param   projectName Name of the project being written to
   * @param   projectData JSON object of the data to be written
   */
  void saveProjectJSON(const std::string &projectName, const nlohmann::json &projectData);

 public:
  /**
   * @brief   Constructs a JSONManager object
   * @details Initializes the JSONManager. The constructor sets up internal
   *          variables and creates the DEFAULT_JSON_PATH folder
   */
  JSONManager();

  /**
   * @brief   Validate if a project JSON exists
   * @param   projectName Name of the project to validate
   * @returns TRUE if the project JSON exists
   *          FALSE if the project JSON does not exist
   */
  bool projectExists(const std::string &projectName);

  /**
   * @brief   Delete an existing project JSON
   * @param   projectName Name of the project to delete
   */
  void deleteProject(const std::string &projectName);

  /**
   * @brief   Sets a value for a given key in the project JSON
   * @details This function loads the project's JSON, updates the value for the given key,
   *          and then saves the updated JSON back to the project file
   * @tparam  T The type of the value to be set
   * @param   projectName Name of the project whose JSON will be updated
   * @param   key The key in the JSON to update
   * @param   value The value to set for the given key
   */
  template <typename T>
  void setProjectValue(const std::string &projectName, const std::string &key, T value) {
    try {
      nlohmann::json projectJSON = loadProjectJSON(projectName);

      projectJSON[key] = value;

      saveProjectJSON(projectName, projectJSON);
    } catch (const std::exception &e) {
      std::cerr << "Error setting project value: " << e.what() << std::endl;
    }
  }

  /**
   * @brief   Sets a nested value for a given key in the project JSON
   * @details This function loads the project's JSON, updates the value for the given key,
   *          and then saves the updated JSON back to the project file
   * @tparam  T The type of the value to be set
   * @param   projectName Name of the project whose JSON will be updated
   * @param   keyPath The key path in the JSON to update in the format: { 'key1', 'key2', 'key3' }
   * @param   value The value to set for the given key
   */
  template <typename T>
  void setProjectNestedValue(const std::string &projectName, const std::vector<std::string> &keyPath, const T &value) {
    try {
      nlohmann::json projectJSON = loadProjectJSON(projectName);

      nlohmann::json *current = &projectJSON;

      /* Navigate to the desired key location */
      for (size_t i = 0; i < keyPath.size() - 1; ++i) {
        if (!current->contains(keyPath[i])) {
          (*current)[keyPath[i]] = nlohmann::json::object();
        }
        /* Update the JSON pointer to the nested JSON */
        current = &((*current)[keyPath[i]]);
      }

      (*current)[keyPath.back()] = value;

      saveProjectJSON(projectName, projectJSON);
    } catch (const std::exception &e) {
      std::cerr << "Error setting nested project value: " << e.what() << std::endl;
    }
  }

  /**
   * @brief   Gets a value for a given key in the project JSON
   * @details This function loads the project's JSON, and then retrieves the value for the given key
   * @tparam  T The type of the value to be fetched
   * @param   projectName Name of the project whose JSON will be checked
   * @param   key The key in the JSON to check
   * @returns Value saved at the JSON key
   */
  template <typename T>
  T getProjectValue(const std::string &projectName, const std::string &key) {
    try {
      nlohmann::json projectJSON = loadProjectJSON(projectName);

      if (projectJSON.contains(key)) {
        return projectJSON[key].get<T>();
      }

    } catch (const std::exception &e) {
      std::cerr << "Error getting project value: " << e.what() << std::endl;
    }
    return static_cast<T>(0U);
  }

  /**
   * @brief   Gets a nested value for a given key in the project JSON
   * @details This function loads the project's JSON, and then retrieves the value for the given key
   * @tparam  T The type of the value to be fetched
   * @param   projectName Name of the project whose JSON will be checked
   * @param   keyPath The key path in the JSON to check in the format: { 'key1', 'key2', 'key3' }
   * @returns Value saved at the JSON key
   */
  template <typename T = nlohmann::json>
  T getProjectNestedValue(const std::string &projectName, const std::vector<std::string> &keyPath, const T &defaultValue = T()) {
    try {
      nlohmann::json projectJSON = loadProjectJSON(projectName);

      const nlohmann::json *current = &projectJSON;
      /* Navigate to the desired key location */
      for (const auto &key : keyPath) {
        if (!current->contains(key)) {
          return defaultValue;
        }
        /* Update the JSON pointer to the nested JSON */
        current = &((*current)[key]);
      }

      return current->get<T>();
    } catch (const std::exception &e) {
      std::cerr << "Error getting nested project value: " << e.what() << std::endl;
      return defaultValue;
    }
  }
};

/** @} */