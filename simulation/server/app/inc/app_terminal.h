#pragma once

/************************************************************************************************
 * @file   app_terminal.h
 *
 * @brief  Header file defining the Application Terminal class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "client_connection.h"
#include "server.h"

/* Intra-component Headers */

/**
 * @defgroup AppTerminal
 * @brief    Application Terminal/Shell Interface
 * @{
 */

/**
 * @class   Terminal
 * @brief   Class that interfaces with the simulation API via a terminal UI
 * @details This class is responsible packaging commands based on terminal inputs
 *          This class is the lightest application-layer, and should be replaced in the future with a GUI
 */
class Terminal {
 private:
  Server *m_Server;                 /**< Pointer to the server instance */
  ClientConnection *m_targetClient; /**< Pointer to a target client instance */

  /**
   * @brief   Convert a string input to lower case
   * @param   input String input to be converted
   * @return  Lower-case version of the input string
   */
  std::string toLower(const std::string &input);

  /**
   * @brief   Handle GPIO commands provoidede an action statement and tokenized parameters
   * @param   action Action statement to select the Remote procedure call
   * @param   tokens List containing action parameters to format the Remote procedure call
   */
  void handleGpioCommands(const std::string &action, std::vector<std::string> &tokens);

  /**
   * @brief   Parse the tokens and branch to the appropiate handler function
   * @details This shall branch to handleGpioCommands, handleI2CCommands, handleSPICommands and handleInterruptCommands
   * @param   tokens List containing tokenized input string
   */
  void parseCommand(std::vector<std::string> &tokens);

 public:
  /**
   * @brief   Constructs a Terminal object
   * @details Initializes the Terminal. The constructor sets up internal variables
   *          The constructor shall bind to the server
   * @param   server Pointer to a server instance
   */
  Terminal(Server *server);

  /**
   * @brief   While true spin-loop to handle shell inputs
   * @details This function must be called after server initialization
   *          This function will never return. To exit safely, you must enter 'exit'
   */
  void run();
};

/** @} */
