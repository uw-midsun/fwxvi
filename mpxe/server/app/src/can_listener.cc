/************************************************************************************************
 * @file   can_message_handler.cc
 *
 * @brief  Source file defining the CanListener Class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <cstring>
#include <iostream>

/* Inter-component Headers */
#include <unistd.h>

#include "system_can.h"
#include "thread_helpers.h"

/* Intra-component Headers */
#include "app.h"
#include "can_listener.h"

#define CAN_MESSAGE_JSON_KEY "messages"

CanListener::CanListener() {
  m_isListening = false;
  m_rawCanSocket = -1;
  pthread_mutex_init(&m_mutex, nullptr);
}

CanListener::~CanListener() {
  pthread_mutex_destroy(&m_mutex);
}

void CanListener::listenCanBusProcedure() {
  m_rawCanSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW);

  if (m_rawCanSocket < 0) {
    throw std::runtime_error("Error creating raw CAN socket");
  }

  struct ifreq ifr;
  strcpy(ifr.ifr_name, CAN_INTERFACE_NAME.c_str());
  if (ioctl(m_rawCanSocket, SIOCGIFINDEX, &ifr) < 0) {
    throw std::runtime_error("Error binding raw CAN socket to interface");
  }

  struct sockaddr_can addr = {};
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;

  if (bind(m_rawCanSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    throw std::runtime_error("Error binding raw CAN socket");
  }

  m_isListening = true;

  struct can_frame canFrame;
  int numBytes;

  while (m_isListening) {
    numBytes = read(m_rawCanSocket, &canFrame, sizeof(struct can_frame));

    if (numBytes < 0) {
      throw std::runtime_error("Error reading CAN data");
      break;
    }

    pthread_mutex_lock(&m_mutex);
    canMessageHandler(canFrame.can_id, canFrame.data);
    pthread_mutex_unlock(&m_mutex);
  }

  close(m_rawCanSocket);
  m_isListening = false;
}

void CanListener::updateJSONProcedure() {
  while (m_isListening) {
    pthread_mutex_lock(&m_mutex);
    serverJSONManager.setProjectValue(CAN_JSON_NAME, CAN_MESSAGE_JSON_KEY, m_canInfo);
    pthread_mutex_unlock(&m_mutex);

    thread_sleep_ms(UPDATE_CAN_JSON_PERIOD_MS);
  }
}

void *listenCanBusWrapper(void *param) {
  CanListener *canListener = static_cast<CanListener *>(param);

  try {
    canListener->listenCanBusProcedure();
  } catch (std::exception &e) {
    std::cerr << "Can Listener Listener Thread Error: " << e.what() << std::endl;
  }

  return nullptr;
}

void *updateJSONWrapper(void *param) {
  CanListener *canListener = static_cast<CanListener *>(param);

  try {
    canListener->updateJSONProcedure();
  } catch (std::exception &e) {
    std::cerr << "Can Listener Update JSON Thread Error: " << e.what() << std::endl;
  }

  return nullptr;
}

void CanListener::listenCanBus() {
  if (m_isListening) return;

  if (pthread_create(&m_listenCanBusId, nullptr, listenCanBusWrapper, this)) {
    throw std::runtime_error("CAN listener thread creation error");
  }

  if (pthread_create(&m_updateJSONId, nullptr, updateJSONWrapper, this)) {
    throw std::runtime_error("CAN update JSON thread creation error");
  }
}
