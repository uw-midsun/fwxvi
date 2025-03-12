/************************************************************************************************
 * @file   client_connection.cc
 *
 * @brief  Source file defining the ClientConnection class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <iostream>

/* Inter-component Headers */
#include <fcntl.h>
#include <unistd.h>

/* Intra-component Headers */
#include "client_connection.h"
#include "server.h"

std::string ClientConnection::getClientAddress() const {
  char ip_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(m_clientAddress.sin_addr), ip_str, INET_ADDRSTRLEN);
  return std::string(ip_str);
}

ClientConnection::ClientConnection(Server *server) {
  this->server = server;
  m_isConnected = false;
}

ClientConnection::~ClientConnection() {
  close(m_clientSocket);
}

bool ClientConnection::acceptClient(int listeningSocket) {
  socklen_t clientLength = sizeof(m_clientAddress);
  m_clientSocket = accept(listeningSocket, (struct sockaddr *)&m_clientAddress, &clientLength);

  if (m_clientSocket < 0) {
    std::cerr << "Failed to accept client connection" << std::endl;
    return false;
  }

  /* Set as non blocking */
  int flags = fcntl(m_clientSocket, F_GETFL, 0);
  fcntl(m_clientSocket, F_SETFL, flags | O_NONBLOCK);

  if (m_clientName.empty()) {
    m_clientName = getClientAddress();
  }

  m_isConnected = true;
  return true;
}

void ClientConnection::sendMessage(const std::string &message) {
  if (!m_isConnected) {
    throw std::runtime_error("Attempting to send on unconnected socket");
  }

  size_t bytesSent = write(m_clientSocket, message.c_str(), message.length());
  if (bytesSent != message.length()) {
    throw std::runtime_error("Failed to send complete message");
  }
}

std::string ClientConnection::getClientName() const {
  return m_clientName;
}

void ClientConnection::setClientName(const std::string &name) {
  m_clientName = name;
}

int ClientConnection::getClientPort() const {
  return m_clientPort;
}

int ClientConnection::getSocketFd() const {
  if (m_clientSocket < 0) {
    throw std::runtime_error("Attempting to get invalid socket descriptor");
  }
  return m_clientSocket;
}

bool ClientConnection::isConnected() {
  return m_isConnected;
}
