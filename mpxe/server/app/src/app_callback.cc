/************************************************************************************************
 * @file   app_callback.cc
 *
 * @brief  Source file defining the Application Callbacks for the server
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <iostream>
#include <string>

/* Inter-component Headers */
#include "command_code.h"
#include "gpio_datagram.h"
#include "i2c_datagram.h"
#include "json_manager.h"
#include "metadata.h"
#include "spi_datagram.h"

/* Intra-component Headers */
#include "app.h"
#include "app_callback.h"

void applicationMessageCallback(Server *server, ClientConnection *client, std::string &message) {
  std::string clientName = client->getClientName();

  auto [commandCode, payload] = decodeCommand(message);
  switch (commandCode) {
    case CommandCode::METADATA: {
      Datagram::Metadata clientMetadata;
      clientMetadata.deserialize(payload);

      if (client->getClientName() != clientMetadata.getProjectName()) {
        server->updateClientName(client, clientMetadata.getProjectName());
      }

      serverJSONManager.setProjectValue(client->getClientName(), "project_name", client->getClientName()); /* Get the updated name if there are duplicates */
      serverJSONManager.setProjectValue(client->getClientName(), "project_status", clientMetadata.getProjectStatus());
      serverJSONManager.setProjectValue(client->getClientName(), "hardware_model", clientMetadata.getHardwareModel());
      break;
    }
    case CommandCode::GPIO_GET_PIN_STATE: {
      serverGpioManager.updateGpioPinState(clientName, payload);
      break;
    }
    case CommandCode::GPIO_GET_ALL_STATES: {
      serverGpioManager.updateGpioAllStates(clientName, payload);
      break;
    }
    case CommandCode::GPIO_GET_PIN_MODE: {
      serverGpioManager.updateGpioPinMode(clientName, payload);
      break;
    }
    case CommandCode::GPIO_GET_ALL_MODES: {
      serverGpioManager.updateGpioAllModes(clientName, payload);
      break;
    }
    case CommandCode::GPIO_GET_PIN_ALT_FUNCTION: {
      serverGpioManager.updateGpioPinAltFunction(clientName, payload);
      break;
    }
    case CommandCode::GPIO_GET_ALL_ALT_FUNCTIONS: {
      serverGpioManager.updateGpioAllAltFunctions(clientName, payload);
      break;
    }
    default: {
      break;
    }
  }
}

void applicationConnectCallback(Server *server, ClientConnection *client) {
  std::cout << "Connected to new client on address: " << client->getClientAddress() << std::endl;
}