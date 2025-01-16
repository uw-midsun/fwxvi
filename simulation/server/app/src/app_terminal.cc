/************************************************************************************************
 * @file   app_terminal.cc
 *
 * @brief  Source file defining the Application Terminal Class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/* Inter-component Headers */
#include "command_code.h"

/* Intra-component Headers */
#include "app.h"
#include "app_terminal.h"

Terminal::Terminal(Server *server) {
  m_Server = server;
}

std::string Terminal::toLower(const std::string &input) {
  std::string lowered = input;
  std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) { return std::tolower(c); });
  return lowered;
}

void Terminal::handleGpioCommands(const std::string &action, std::vector<std::string> &tokens) {
  std::string message;
  if (action == "get_pin_state" && tokens.size() >= 3) {
    message = serverGpioManager.createGpioCommand(CommandCode::GPIO_GET_PIN_STATE, tokens[2], "");
  } else if (action == "get_all_states" && tokens.size() >= 2) {
    message = serverGpioManager.createGpioCommand(CommandCode::GPIO_GET_ALL_STATES, tokens[0], "");
  } else if (action == "get_pin_mode" && tokens.size() >= 3) {
    message = serverGpioManager.createGpioCommand(CommandCode::GPIO_GET_PIN_MODE, tokens[2], "");
  } else if (action == "get_all_modes" && tokens.size() >= 2) {
    message = serverGpioManager.createGpioCommand(CommandCode::GPIO_GET_ALL_MODES, tokens[0], "");
  } else if (action == "get_pin_alt_function" && tokens.size() >= 3) {
    message = serverGpioManager.createGpioCommand(CommandCode::GPIO_GET_PIN_ALT_FUNCTION, tokens[2], "");
  } else if (action == "get_all_alt_functions" && tokens.size() >= 2) {
    message = serverGpioManager.createGpioCommand(CommandCode::GPIO_GET_ALL_ALT_FUNCTIONS, tokens[0], "");
  } else if (action == "set_pin_state" && tokens.size() >= 4) {
    message = serverGpioManager.createGpioCommand(CommandCode::GPIO_SET_PIN_STATE, tokens[2], tokens[3]);
  } else if (action == "set_all_states" && tokens.size() >= 3) {
    message = serverGpioManager.createGpioCommand(CommandCode::GPIO_SET_ALL_STATES, tokens[0], tokens[2]);
  } else {
    std::cerr << "Unsupported action: " << action << std::endl;
  }

  if (!message.empty()) {
    m_Server->sendMessage(m_targetClient, message);
  } else {
    std::cout << "Invalid command. Refer to sim_command.md" << std::endl;
  }
  m_targetClient = nullptr;
}

void Terminal::parseCommand(std::vector<std::string> &tokens) {
  if (tokens.size() < 2) {
    std::cout << "Invalid command. Format: <interface> <action> <args...>\n";
    return;
  }

  std::string interface = toLower(tokens[0]);
  std::string action = toLower(tokens[1]);

  try {
    if (interface == "gpio") {
      handleGpioCommands(action, tokens);
    } else if (interface == "i2c") {
    } else if (interface == "spi") {
    } else {
      std::cerr << "Unsupported interface: " << interface << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "Terminal command error: " << e.what() << std::endl;
  }
}

void Terminal::run() {
  std::string input;

  while (true) {
    std::cout << "Client List:" << std::endl;
    std::cout << "------------" << std::endl;
    m_Server->dumpClientList();
    std::cout << "------------" << std::endl;

    std::cout << "Select Client by Name (Enter to refresh) > ";
    std::getline(std::cin, input);

    input.erase(0, input.find_first_not_of(" \t"));
    input.erase(input.find_last_not_of(" \t") + 1);

    if (toLower(input) == "exit") {
      break;
    }

    m_targetClient = m_Server->getClientByName(input);

    if (m_targetClient == nullptr) {
      std::cout << "Invalid client selection." << std::endl << std::endl;
      continue;
    }
    std::cout << "Selected " << m_targetClient->getClientName() << std::endl << std::endl;

    std::cout << "Enter commmand > ";
    std::getline(std::cin, input);

    input.erase(0, input.find_first_not_of(" \t"));
    input.erase(input.find_last_not_of(" \t") + 1);

    if (toLower(input) == "exit") {
      break;
    }

    /**
     * Create tokens out of input string
     * Input = "GPIO GET_PIN_STATE A9"
     * tokens = ["GPIO", "GET_PIN_STATE", "A9"]
     */
    std::vector<std::string> tokens;
    std::istringstream iss(input);
    std::string token;
    while (iss >> token) {
      tokens.push_back(token);
    }

    if (!tokens.empty()) {
      parseCommand(tokens);
    }
  }
}
