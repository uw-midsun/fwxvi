#pragma once

/************************************************************************************************
 * @file   command_code.h
 *
 * @brief  Header file defining the CommandCodes
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <string>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup CommandCodes
 * @brief    Commands Supported by the Simulation
 * @{
 */

/**
 * @brief   Command Code Class
 */
enum class CommandCode {
  /* MISC Commands */
  METADATA, /**< Retrieve Client Metadata Command */

  /* GPIO Commands */
  GPIO_SET_PIN_STATE,         /**< Set a Gpio Pin state */
  GPIO_SET_ALL_STATES,        /**< Set all Gpio Pin states */
  GPIO_GET_PIN_STATE,         /**< Get a Gpio Pin state */
  GPIO_GET_ALL_STATES,        /**< Get all Gpio Pin states */
  GPIO_GET_PIN_MODE,          /**< Get a Gpio Pin mode */
  GPIO_GET_ALL_MODES,         /**< Get all Gpio Pin modes */
  GPIO_GET_PIN_ALT_FUNCTION,  /**< Get a Gpio Pin alternate function */
  GPIO_GET_ALL_ALT_FUNCTIONS, /**< Get all Gpio Pin alternate functions */

  /* I2C Commands */

  /* SPI Commands */

  /* UART Commands */

  /* FLASH Commands */

  /* ADC Commands */
  ADC_SET_RAW,           /**< Set the raw reading of an ADC Channel */
  ADC_SET_CONVERTED,     /**< Set the converted reading of an ADC Channel */
  ADC_SET_ALL_RAW,       /**< Set the raw reading of all ADC Channels */
  ADC_SET_ALL_CONVERTED, /**< Set the converted reading of all ADC Channels */
  ADC_GET_RAW,           /**< Get the raw reading of an ADC Channel */
  ADC_GET_ALL_RAW,       /**< Get the raw reading of all ADC Channels */
  ADC_GET_CONVERTED,     /**< Get the converted reading of an ADC Channel */
  ADC_GET_ALL_CONVERTED, /**< Get the converted reading of all ADC Channels */

  NUM_COMMAND_CODES /**< Number of command codes */
};

/**
 * @brief   Encode a command code into an existing data payload
 * @param   commandCode Command code to be encoded
 * @param   message String data payload to be modified
 * @return  Encoded message
 */
std::string encodeCommand(const CommandCode commandCode, std::string &message);

/**
 * @brief   Decode a command code from existing data
 * @param   message String data payload to be decoded
 * @return  Pair containing <CommandCode, Data payload without the command Code>
 */
std::pair<CommandCode, std::string> decodeCommand(std::string &message);

/** @} */
