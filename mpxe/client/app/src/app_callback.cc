/************************************************************************************************
 * @file   app_callback.cc
 *
 * @brief  Source file defining the Application Callbacks for the client
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "command_code.h"
#include "json_manager.h"
#include "metadata.h"

/* Intra-component Headers */
#include "app.h"
#include "app_callback.h"

GpioManager clientGpioManager;
AfeManager clientAfeManager;

void applicationMessageCallback(Client *client, std::string &message) {
  std::string data = message;
  auto [commandCode, payload] = decodeCommand(message);

  switch (commandCode) {
    case CommandCode::METADATA: {
      /* Future expansion if the server needs to send the client some metadata? */
      break;
    }
    case CommandCode::GPIO_SET_PIN_STATE: {
      clientGpioManager.setGpioPinState(payload);
      break;
    }
    case CommandCode::GPIO_SET_ALL_STATES: {
      clientGpioManager.setGpioAllStates(payload);
      break;
    }
    case CommandCode::GPIO_GET_PIN_STATE: {
      client->sendMessage(clientGpioManager.processGpioPinState(payload));
      break;
    }
    case CommandCode::GPIO_GET_ALL_STATES: {
      client->sendMessage(clientGpioManager.processGpioAllStates());
      break;
    }
    case CommandCode::GPIO_GET_PIN_MODE: {
      client->sendMessage(clientGpioManager.processGpioPinMode(payload));
      break;
    }
    case CommandCode::GPIO_GET_ALL_MODES: {
      client->sendMessage(clientGpioManager.processGpioAllModes());
      break;
    }
    case CommandCode::GPIO_GET_PIN_ALT_FUNCTION: {
      client->sendMessage(clientGpioManager.processGpioPinAltFunction(payload));
      break;
    }
    case CommandCode::GPIO_GET_ALL_ALT_FUNCTIONS: {
      client->sendMessage(clientGpioManager.processGpioAllAltFunctions());
      break;
    }
    case CommandCode::AFE_SET_CELL: {
      clientAfeManager.setAfeCell(payload);
      break;
    }
    case CommandCode::AFE_SET_THERMISTOR: {
      clientAfeManager.setAfeTherm(payload);
      break;
    }
    case CommandCode::AFE_SET_DEV_CELL: {
      clientAfeManager.setAfeDevCell(payload);
      break;
    }
    case CommandCode::AFE_SET_DEV_THERMISTOR: {
      clientAfeManager.setAfeDevTherm(payload);
      break;
    }
    case CommandCode::AFE_SET_PACK_CELL: {
      clientAfeManager.setAfePackCell(payload);
      break;
    }
    case CommandCode::AFE_SET_PACK_THERMISTOR: {
      clientAfeManager.setAfePackTherm(payload);
      break;
    }
    case CommandCode::AFE_SET_BOARD_TEMP: {
      clientAfeManager.setAfeBoardTherm(payload); 
      break;
    }
    case CommandCode::AFE_SET_DISCHARGE: {
      clientAfeManager.setCellDischarge(payload);
      break;
    }
    case CommandCode::AFE_SET_PACK_DISCHARGE: {
      clientAfeManager.setCellPackDischarge(payload);
      break;
    }
    case CommandCode::AFE_GET_CELL: {
      client->sendMessage(clientAfeManager.processAfeCell(payload));
      break;
    }
    case CommandCode::AFE_GET_THERMISTOR: {
      client->sendMessage(clientAfeManager.processAfeTherm(payload));
      break;
    }
    case CommandCode::AFE_GET_DEV_CELL: {
      client->sendMessage(clientAfeManager.processAfeDevCell(payload));
      break;
    }
    case CommandCode::AFE_GET_DEV_THERMISTOR: {
      client->sendMessage(clientAfeManager.processAfeDevTherm(payload));
      break;
    }
    case CommandCode::AFE_GET_PACK_CELL: {
      client->sendMessage(clientAfeManager.processAfePackCell());
      break;
    }
    case CommandCode::AFE_GET_PACK_THERMISTOR: {
      client->sendMessage(clientAfeManager.processAfePackTherm());
      break;
    }
    case CommandCode::AFE_GET_DISCHARGE: {
      client->sendMessage(clientAfeManager.processCellDischarge(payload));
      break;
    }
    case CommandCode::AFE_GET_PACK_DISCHARGE: {
      client->sendMessage(clientAfeManager.processCellPackDischarge());
      break;
    }
    case CommandCode::AFE_GET_BOARD_TEMP: {
      client->sendMessage(clientAfeManager.processAfeBoardTherm(payload));
      break;
    }
    default: {
      break;
    }
  }
}

void applicationConnectCallback(Client *client) {
  std::cout << "Connected :-)" << std::endl;

  std::string projectName = DEFAULT_PROJECT_NAME;
  std::string hardwareModel = DEFAULT_HARDWARE_MODEL;

  if (!client->getClientName().empty()) {
    projectName = client->getClientName();
  }

  Datagram::Metadata::Payload initialData = { .projectName = projectName, .projectStatus = "RUNNING", .hardwareModel = hardwareModel };
  Datagram::Metadata projectMetadata(initialData);

  client->sendMessage(projectMetadata.serialize());
}
