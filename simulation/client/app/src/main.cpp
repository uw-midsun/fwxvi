#include <iostream>

#include "app.h"
#include "app_callback.h"
#include "gpio_manager.h"
#include "metadata.h"
#include "ntp_client.h"
#include "tcp_client.h"

GpioManager clientGpioManager;
std::string projectName = DEFAULT_PROJECT_NAME;

void connectCallback(TCPClient *client) {
  Datagram::Metadata::Payload initialData = {.projectName = projectName, .projectStatus = "RUNNING", .hardwareModel = "MS16.0.0"};

  Datagram::Metadata projectMetadata(initialData);

  client->sendMessage(projectMetadata.serialize());
}

int main(int argc, char **argv) {
  std::cout << "Running Client" << std::endl;

  TCPClient serverClient("127.0.0.1", 1024, applicationMessageCallback, connectCallback);
  serverClient.connectServer();

#if USE_NETWORK_TIME_PROTOCOL == 1U
  NTPClient ntpClient;
  ntpClient.startSynchronization("127.0.0.1");
#endif

  int n;
  std::cin >> n;

  return 0;
}
