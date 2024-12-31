
#include <iostream>
#include <string>

#include "app.h"
#include "app_callback.h"
#include "app_terminal.h"
#include "client_connection.h"
#include "gpio_manager.h"
#include "json_manager.h"
#include "ntp_server.h"
#include "tcp_server.h"

JSONManager globalJSON;
GpioManager serverGpioManager;

int main(int argc, char **argv) {
  std::cout << "Running Server" << std::endl;
  TCPServer tcp_server;
  Terminal application_terminal(&tcp_server);

  tcp_server.listenClients(1024, applicationCallback);

#if USE_NETWORK_TIME_PROTOCOL == 1U
  ntp_server.startListening("127.0.0.1", "time.google.com");
  NTPServer ntp_server;
#endif

  application_terminal.run();

  return 0;
}
