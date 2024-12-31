#include "app_callback.h"

#include <iostream>

#include "app.h"
#include "command_code.h"
#include "gpio_datagram.h"
#include "i2c_datagram.h"
#include "json_manager.h"
#include "metadata.h"
#include "spi_datagram.h"

void applicationCallback(TCPServer *srv, ClientConnection *src, std::string &message) {
  std::string clientName = src->getClientName();

  auto [commandCode, payload] = decodeCommand(message);
  switch (commandCode) {
    case CommandCode::METADATA: {
      Datagram::Metadata clientMetadata;
      clientMetadata.deserialize(payload);

      if (src->getClientName() != clientMetadata.getProjectName()) {
        srv->updateClientName(src, clientMetadata.getProjectName());
      }

      globalJSON.setProjectValue(src->getClientName(), "project_name", src->getClientName()); /* Get the updated name if there are duplicates */
      globalJSON.setProjectValue(src->getClientName(), "project_status", clientMetadata.getProjectStatus());
      globalJSON.setProjectValue(src->getClientName(), "hardware_model", clientMetadata.getHardwareModel());

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