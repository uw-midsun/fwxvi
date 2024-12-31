#include "json_manager.h"

#include <chrono>
#include <ctime>

void JSONManager::createDefaultProjectJSON(const std::string &projectName) {
  if (projectExists(projectName)) {
    std::cerr << "Project '" << projectName << "' already exists." << std::endl;
    return;
  }

  try {
    std::time_t now = std::time(nullptr);

    /* YYYY-MM-DD HH:MM:SS -> 19 chars + null terminator */
    char timeBuffer[20];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

    nlohmann::json defaultJSON = {
        {"project_name", projectName}, {"version", "1.0.0"}, {"created_at", std::string(timeBuffer)}, {"settings", nlohmann::json::object()}};
    saveProjectJSON(projectName, defaultJSON);
  } catch (const std::exception &e) {
    std::cerr << "Error creating project JSON: " << e.what() << std::endl;
  }
}

std::filesystem::path JSONManager::getProjectFilePath(const std::string &projectName) {
  std::string sanitizedName = projectName;

  /* Get rid of special characters that aren't _ or - */
  for (char &c : sanitizedName) {
    if (!std::isalnum(c) && c != '_' && c != '-') {
      c = '_';
    }
  }
  return m_projectBasePath / (sanitizedName + ".json");
}

nlohmann::json JSONManager::loadProjectJSON(const std::string &projectName) {
  try {
    std::filesystem::path projectPath = getProjectFilePath(projectName);

    if (!std::filesystem::exists(projectPath)) {
      createDefaultProjectJSON(projectName);
    }

    std::ifstream projectFile(projectPath);
    if (!projectFile.is_open()) {
      throw std::runtime_error("Could not open project JSON file");
    }

    return nlohmann::json::parse(projectFile);

  } catch (const std::exception &e) {
    std::cerr << "Error loading project JSON: " << e.what() << std::endl;
  }
  return nlohmann::json::object();
}

void JSONManager::saveProjectJSON(const std::string &projectName, const nlohmann::json &projectData) {
  try {
    std::filesystem::path projectPath = getProjectFilePath(projectName);

    std::ofstream projectFile(projectPath);
    if (!projectFile.is_open()) {
      throw std::runtime_error("Could not open project JSON file for writing");
    }

    projectFile << projectData.dump(2);
  } catch (const std::exception &e) {
    std::cerr << "Error saving project JSON: " << e.what() << std::endl;
  }
}

JSONManager::JSONManager() {
  /* Create the JSON output directory */
  m_projectBasePath = std::filesystem::absolute(DEFAULT_JSON_PATH);
  std::filesystem::create_directories(m_projectBasePath);

  /* Clean up the directory by deleting all .json files */
  for (const auto &file : std::filesystem::directory_iterator(m_projectBasePath)) {
    if (file.is_regular_file() && file.path().extension() == ".json") {
      std::filesystem::remove(file.path());
    }
  }
}

bool JSONManager::projectExists(const std::string &projectName) {
  return std::filesystem::exists(getProjectFilePath(projectName));
}

void JSONManager::deleteProject(const std::string &projectName) {
  try {
    std::filesystem::path projectPath = getProjectFilePath(projectName);

    if (std::filesystem::exists(projectPath)) {
      std::filesystem::remove(projectPath);
    } else {
      std::cerr << "Project '" << projectName << "' does not exist." << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "Error deleting project JSON: " << e.what() << std::endl;
  }
}
