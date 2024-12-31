#ifndef JSON_MANAGER_H
#define JSON_MANAGER_H

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

#define PROJECT_VERSION "1.0.0"

class JSONManager {
 private:
  static constexpr const char *DEFAULT_JSON_PATH = "./Simulation_JSON/";
  std::filesystem::path m_projectBasePath;

  void createDefaultProjectJSON(const std::string &projectName);
  std::filesystem::path getProjectFilePath(const std::string &projectName);

  nlohmann::json loadProjectJSON(const std::string &projectName);
  void saveProjectJSON(const std::string &projectName, const nlohmann::json &projectData);

 public:
  JSONManager();

  bool projectExists(const std::string &projectName);
  void deleteProject(const std::string &projectName);

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

#endif
