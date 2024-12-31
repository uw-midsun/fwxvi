#ifndef COMMAND_CODE_H
#define COMMAND_CODE_H

#include <string>

enum class CommandCode {
  /* MISC Commands */
  METADATA,

  /* GPIO Commands */
  GPIO_SET_PIN_STATE,
  GPIO_SET_ALL_STATES,
  GPIO_GET_PIN_STATE,
  GPIO_GET_ALL_STATES,
  GPIO_GET_PIN_MODE,
  GPIO_GET_ALL_MODES,
  GPIO_GET_PIN_ALT_FUNCTION,
  GPIO_GET_ALL_ALT_FUNCTIONS,

  /* I2C Commands */

  /* SPI Commands */

  /* UART Commands */

  /* FLASH Commands */

  NUM_COMMAND_CODES
};

std::string encodeCommand(const CommandCode commandCode, std::string &message);
std::pair<CommandCode, std::string> decodeCommand(std::string &message);

#endif
