#ifndef CAN_MANAGER_H
#define CAN_MANAGER_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/can.h>
#include <linux/can/bcm.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

class CanManager {
 private:
  const std::string CAN_INTERFACE_NAME = "vcan0";
  static const constexpr int UPDATE_CAN_JSON_FREQUENCY_MS = 1000U;

  // std::unordered_map<> m_canInfo;

  pthread_mutex_t m_mutex;
  pthread_t m_listenCanBusId;

  int m_listeningSocket;
  struct sockaddr_can m_canAddress;
  struct ifreq m_interfaceRequest;

  std::atomic<bool> m_isListening;

  void canCallback();

 public:
  CanManager() = default;

  void listenCanBus();
  void listenCanBusProcedure();
};

#endif
