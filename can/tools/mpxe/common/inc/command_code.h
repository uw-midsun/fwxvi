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
  ADC_SET_ALL_RAW,       /**< Set the raw reading of all ADC Channels */
  ADC_GET_RAW,           /**< Get the raw reading of an ADC Channel */
  ADC_GET_ALL_RAW,       /**< Get the raw reading of all ADC Channels */
  ADC_GET_CONVERTED,     /**< Get the converted reading of an ADC Channel */
  ADC_GET_ALL_CONVERTED, /**< Get the converted reading of all ADC Channels */

  /* ADBMS AFE commands */
  AFE_SET_CELL,            /**< Set single cell voltage */
  AFE_SET_THERMISTOR,      /**< Set single thermistor voltage */
  AFE_SET_DEV_CELL,        /**< Set cell voltages for all cells in device */
  AFE_SET_DEV_THERMISTOR,  /**< Set thermistor voltages for all thermistor in device */
  AFE_SET_PACK_CELL,       /**< Set cell voltages for all cells */
  AFE_SET_PACK_THERMISTOR, /**< Set thermistor voltages for all thermistor */
  AFE_SET_DISCHARGE,       /**< Set single cell discharge */
  AFE_SET_PACK_DISCHARGE,  /**< Set discharge for whole pack */
  AFE_SET_BOARD_TEMP,      /**< Set board thermistor voltage */
  AFE_GET_CELL,            /**< Get a single cell voltage reading */
  AFE_GET_THERMISTOR,      /**< Get a single thermistor voltage reading */
  AFE_GET_DEV_CELL,        /**< Get cell voltages for all cells in device */
  AFE_GET_DEV_THERMISTOR,  /**< Get thermistor voltages for all thermistor in device */
  AFE_GET_PACK_CELL,       /**< Get cell voltages for all cells */
  AFE_GET_PACK_THERMISTOR, /**< Get thermistor voltages for all thermistor */
  AFE_GET_DISCHARGE,       /**< Get single cell discharge */
  AFE_GET_PACK_DISCHARGE,  /**< Get Discharge for whole pack */
  AFE_GET_BOARD_TEMP,      /**< Get board thermistor voltage */

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
