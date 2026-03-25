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
#include <nlohmann/json.hpp>

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
    std::cout << "Invalid command. Refer to command.md" << std::endl;
  }
  m_targetClient = nullptr;
}

void Terminal::handleAfeCommands(const std::string &action, std::vector<std::string> &tokens) {
  std::string message;
  if (action == "set_cell" && tokens.size() >= 4) {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_SET_CELL, tokens[2], tokens[3]);
  } else if (action == "set_thermistor" && tokens.size() >= 4) {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_SET_THERMISTOR, tokens[2], tokens[3]);
  } else if (action == "set_dev_cell" && tokens.size() >= 4) {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_SET_DEV_CELL, tokens[2], tokens[3]);
  } else if (action == "set_dev_thermistor" && tokens.size() >= 4) {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_SET_DEV_THERMISTOR, tokens[2], tokens[3]);
  } else if (action == "set_pack_cell" && tokens.size() >= 3) {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_SET_PACK_CELL, "-1", tokens[2]);
  } else if (action == "set_pack_thermistor" && tokens.size() >= 3) {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_SET_PACK_THERMISTOR, "-1", tokens[2]);
  } else if (action == "set_discharge" && tokens.size() >= 4) {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_SET_DISCHARGE, tokens[2], tokens[3]);
  } else if (action == "set_pack_discharge" && tokens.size() >= 3) {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_SET_PACK_DISCHARGE, "-1", tokens[2]);
  } else if (action == "set_board_temp" && tokens.size() >= 4) {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_SET_BOARD_TEMP, tokens[2], tokens[3]);
  } else if (action == "get_cell" && tokens.size() >= 3) {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_GET_CELL, tokens[2], "");
  } else if (action == "get_thermistor" && tokens.size() >= 3) {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_GET_THERMISTOR, tokens[2], "");
  } else if (action == "get_dev_cell" && tokens.size() >= 3) {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_GET_DEV_CELL, tokens[2], "");
  } else if (action == "get_dev_thermistor" && tokens.size() >= 3) {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_GET_DEV_THERMISTOR, tokens[2], "");
  } else if (action == "get_pack_cell") {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_GET_PACK_CELL, "-1", "");
  } else if (action == "get_pack_thermistor") {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_GET_PACK_THERMISTOR, "-1", "");
  } else if (action == "get_discharge" && tokens.size() >= 3) {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_GET_DISCHARGE, tokens[2], "");
  } else if (action == "get_pack_discharge") {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_GET_PACK_DISCHARGE, "-1", "");
  } else if (action == "get_board_temp" && tokens.size() >= 3) {
    message = serverAfeManager.createAfeCommand(CommandCode::AFE_GET_BOARD_TEMP, tokens[2], "");
  } else {
    std::cerr << "Unsupported action: " << action << std::endl;
  }

  if (!message.empty()) {
    m_Server->sendMessage(m_targetClient, message);
  } else {
    std::cout << "Invalid command. Refer to command.md" << std::endl;
  }
  m_targetClient = nullptr;
}

void Terminal::handleAdcCommands(const std::string &action, std::vector<std::string> &tokens) {
  std::string message;

  if (action == "set_raw" && tokens.size() >= 4) {
    message = serverAdcManager.createAdcCommand(CommandCode::ADC_SET_RAW, tokens[2], tokens[3]);
  } else if (action == "set_all_raw" && tokens.size() >= 3) {
    message = serverAdcManager.createAdcCommand(CommandCode::ADC_SET_ALL_RAW, "", tokens[2]);
  } else if (action == "get_raw" && tokens.size() >= 3) {
    message = serverAdcManager.createAdcCommand(CommandCode::ADC_GET_RAW, tokens[2], "");
  } else if (action == "get_all_raw" && tokens.size() >= 2) {
    message = serverAdcManager.createAdcCommand(CommandCode::ADC_GET_ALL_RAW, "", "");
  } else if (action == "get_converted" && tokens.size() >= 3) {
    message = serverAdcManager.createAdcCommand(CommandCode::ADC_GET_CONVERTED, tokens[2], "");
  } else if (action == "get_all_converted" && tokens.size() >= 2) {
    message = serverAdcManager.createAdcCommand(CommandCode::ADC_GET_ALL_CONVERTED, "", "");
  } else {
    std::cerr << "Unsupported action: " << action << std::endl;
  }

  if (!message.empty()) {
    m_Server->sendMessage(m_targetClient, message);
  } else {
    std::cout << "Invalid ADC command. Refer to command.md" << std::endl;
  }

  m_targetClient = nullptr;
}

void Terminal::handleSpiCommands(const std::string &action, std::vector<std::string> &tokens) {
  std::string message;

  std::string dataStr = "";
  if (tokens.size() >= 4) {
    for (size_t i = 3; i < tokens.size(); ++i) {
      dataStr += tokens[i];
    }
  }

  if (action == "write" && tokens.size() >= 4) {
    message = serverSPIManager.createSpiCommand(CommandCode::SPI_WRITE_DATA, tokens[2], dataStr);
  } else if (action == "read" && tokens.size() >= 3) {
    message = serverSPIManager.createSpiCommand(CommandCode::SPI_READ_DATA, tokens[2], "");
  } else if (action == "transfer" && tokens.size() >= 4) {
    message = serverSPIManager.createSpiCommand(CommandCode::SPI_TRANSFER_DATA, tokens[2], dataStr);
  } else if (action == "clear_buffer" && tokens.size() >= 3) {
    message = serverSPIManager.createSpiCommand(CommandCode::SPI_CLEAR_BUFFER, tokens[2], "");
  } else {
    std::cerr << "Unsupported SPI action: " << action << std::endl;
  }

  if (!message.empty()) {
    m_Server->sendMessage(m_targetClient, message);
  } else {
    std::cout << "Invalid SPI command. Refer to command.md" << std::endl;
  }
  m_targetClient = nullptr;
}

void Terminal::handleCanCommands(const std::string &action, std::vector<std::string> &tokens) {
  // todo: start/stop commands; need proper thread shutdown first
  // if (action == "start") {
  //   serverCanManager.startSimulation();
  // } else if (action == "stop") {
  //   serverCanManager.stopSimulation();
  // } else
  if (action == "list_messages") {
    std::vector<CanMessageInfo> messages = serverCanManager.getMessageInfoList();
    std::cout << "CAN Messages (" << messages.size() << " total):" << std::endl;
    for (unsigned int i = 0; i < messages.size(); i++) {
      std::cout << "  " << messages[i].name << " [" << messages[i].sender
                << ", " << messages[i].cycle_speed << "ms, DLC:" << messages[i].dlc << "]" << std::endl;
    }
  } else if (action == "list_signals" && tokens.size() >= 3) {
    std::string msg_name = toLower(tokens[2]);
    CanMessageInfo info = serverCanManager.getMessageInfo(msg_name);
    if (info.name.empty()) {
      std::cerr << "Unknown message: " << msg_name << std::endl;
    } else {
      std::cout << "Signals for " << info.name << ":" << std::endl;
      for (unsigned int i = 0; i < info.signals.size(); i++) {
        std::cout << "  " << info.signals[i].name << " (" << info.signals[i].length << " bits)" << std::endl;
      }
    }
  } else if (action == "get" && tokens.size() >= 4) {
    std::string msg_name = toLower(tokens[2]);
    std::string sig_name = toLower(tokens[3]);
    uint64_t value = serverCanManager.getSignalByName(msg_name, sig_name);
    std::cout << msg_name << "." << sig_name << " = " << value << std::endl;
  } else if (action == "set" && tokens.size() >= 5) {
    std::string msg_name = toLower(tokens[2]);
    std::string sig_name = toLower(tokens[3]);
    uint64_t value = std::stoull(tokens[4]);
    if (serverCanManager.setSignalByName(msg_name, sig_name, value)) {
      std::cout << "Set " << msg_name << "." << sig_name << " = " << value << std::endl;
    }
  // todo: dump command for raw JSON maybe for the GUI
  // } else if (action == "dump") {
  //   nlohmann::json signals = serverCanManager.getAllSignals();
  //   std::cout << signals.dump(2) << std::endl;
  } else {
    std::cerr << "Unsupported CAN action: " << action << ". Refer to command.md" << std::endl;
  }
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
    } else if (interface == "afe") {
      handleAfeCommands(action, tokens);
    } else if (interface == "adc") {
      handleAdcCommands(action, tokens);
    } else if (interface == "can") {
      handleCanCommands(action, tokens);
    } else if (interface == "i2c") {
    } else if (interface == "spi") {
      handleSpiCommands(action, tokens);
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

    if (toLower(input) == "quit") {
      break;
    }

    /* CAN commands run server side, bypass client selection */
    if (input.length() >= 3 && toLower(input.substr(0, 3)) == "can") {
      std::vector<std::string> tokens;
      std::istringstream ss(input);
      std::string token;
      while (ss >> token) {
        tokens.push_back(token);
      }
      parseCommand(tokens);
      continue;
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

    if (toLower(input) == "quit") {
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
