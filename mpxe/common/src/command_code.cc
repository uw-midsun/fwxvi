/************************************************************************************************
 * @file   command_code.cc
 *
 * @brief  Source file defining the CommandCode helpers
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <iostream>
#include <cstdint>

/* Inter-component Headers */

/* Intra-component Headers */
#include "command_code.h"

std::string encodeCommand(const CommandCode commandCode, std::string &message) {
  return std::to_string(static_cast<uint8_t>(commandCode)) + '|' + message;
}

std::pair<CommandCode, std::string> decodeCommand(std::string &message) {
  size_t delimPosition = message.find('|');

  if (delimPosition == std::string::npos) {
    throw std::runtime_error("Invalid command format");
  }

  uint8_t commandCodeValue = std::stoi(message.substr(0, delimPosition));

  if (commandCodeValue > static_cast<uint8_t>(CommandCode::NUM_COMMAND_CODES)) {
    throw std::runtime_error("CommandCode out of valid range");
  }

  CommandCode commandCode = static_cast<CommandCode>(commandCodeValue);

  std::string payload = message.substr(delimPosition + 1);

  return { commandCode, payload };
}
