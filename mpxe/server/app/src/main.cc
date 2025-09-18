/************************************************************************************************
 * @file   main.cc
 *
 * @brief  Main source file
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <iostream>
#include <string>

/* Inter-component Headers */
#include "client_connection.h"
#include "json_manager.h"
#include "ntp_server.h"
#include "server.h"

/* Intra-component Headers */
#include "adbms_afe_manager.h"
#include "app.h"
#include "app_callback.h"
#include "app_terminal.h"
#include "can_listener.h"
#include "can_scheduler.h"
#include "gpio_manager.h"

JSONManager serverJSONManager;
GpioManager serverGpioManager;
AfeManager serverAfeManager;
CanListener serverCanListener;
CanScheduler serverCanScheduler;

int main(int argc, char **argv) {
  std::cout << "Running Server" << std::endl;
  Server Server;
  Terminal applicationTerminal(&Server);

  Server.listenClients(8080, applicationMessageCallback, applicationConnectCallback);

#if USE_NETWORK_TIME_PROTOCOL == 1U
  ntp_server.startListening("127.0.0.1", "time.google.com");
  NTPServer ntp_server;
#endif

  applicationTerminal.run();

  return 0;
}
