#pragma once

/************************************************************************************************
 * @file   app_callback.h
 *
 * @brief  Header file defining the Application Callback functions for the client
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <string>

/* Inter-component Headers */
#include "client.h"

/* Intra-component Headers */

/**
 * @defgroup ClientAppCallback
 * @brief    Application Callback functions for the Client
 * @{
 */

/**
 * @brief   Handle receiving new client messages
 * @details This shall handle all CommandCodes received
 *          This shall branch out to the GpioManager, I2CManager, SPIManager or InterruptManager
 *          based on the CommandCode
 * @param   client Pointer to the connected client instance
 * @param   message String message value that has been received
 */
void applicationMessageCallback(Client *client, std::string &message);

/**
 * @brief   Handle connecting to the server
 * @details This shall notify the user about a new server connection
 * @param   client Pointer to the connected client instance
 */
void applicationConnectCallback(Client *client);

/** @} */
