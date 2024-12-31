#include "can_manager.h"

#include <unistd.h>

#include <cstring>
#include <iostream>

void CanManager::listenCanBusProcedure() {
  m_listeningSocket = socket(PF_CAN, SOCK_DGRAM, CAN_BCM);

  if (m_listeningSocket < 0) {
    throw std::runtime_error("Error creating socket for CAN Broadcast Manager");
  }

  strcpy(m_interfaceRequest.ifr_name, CAN_INTERFACE_NAME.c_str());
  if (ioctl(m_listeningSocket, SIOCGIFINDEX, &m_interfaceRequest) < 0) {
    throw std::runtime_error("Error writing interface name to socketCAN file descriptor");
  }

  m_canAddress.can_family = AF_CAN;
  m_canAddress.can_ifindex = m_interfaceRequest.ifr_ifindex;

  if (connect(m_listeningSocket, (struct sockaddr *)&m_canAddress, sizeof(m_canAddress)) < 0) {
    throw std::runtime_error("Error connecting to SocketCAN broadcast manager");
  }

  m_isListening = true;

  struct can_frame canFrame;
  int numBytes;

  while (m_isListening) {
    numBytes = read(m_listeningSocket, &canFrame, sizeof(struct can_frame));

    if (numBytes < 0) {
      throw std::runtime_error("Error reading CAN data");
      break;
    }
  }

  close(m_listeningSocket);
  m_isListening = false;
}

void *listenCanBusWrapper(void *param) {
  CanManager *canManager = static_cast<CanManager *>(param);

  try {
    canManager->listenCanBusProcedure();
  } catch (...) {
    std::cerr << "Can Manager Thread Error" << std::endl;
  }

  return nullptr;
}

void CanManager::listenCanBus() {
  if (m_isListening)
    return;

  if (pthread_create(&m_listenCanBusId, nullptr, listenCanBusWrapper, this)) {
    throw std::runtime_error("CAN listener thread creation error");
  }
}

void CanManager::canCallback() {}
