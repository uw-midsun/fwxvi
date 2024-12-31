#include "client_connection.h"

#include <fcntl.h>
#include <unistd.h>

#include <iostream>

#include "tcp_server.h"

std::string ClientConnection::getClientAddress() const {
  char ip_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(m_clientAddress.sin_addr), ip_str, INET_ADDRSTRLEN);
  return std::string(ip_str);
}

ClientConnection::ClientConnection(TCPServer *server) {
  this->server = server;
  m_isConnected = false;
}

ClientConnection::~ClientConnection() {}

void ClientConnection::monitorThreadProcedure() {
  const int bufferSize = MAX_CLIENT_BUFFER_SIZE;
  char buffer[bufferSize + 1];
  int numBytes;

  m_isConnected = true;
  while (true) {
    numBytes = read(m_clientSocket, &buffer, bufferSize);
    if (numBytes <= 0) {
      break;
    }

    buffer[numBytes] = '\0';

    std::string msg(buffer, numBytes);
    server->messageReceived(this, msg);
  }

  close(m_clientSocket);
  m_isConnected = false;

  server->removeClient(this);
}

void *monitorThreadWrapper(void *param) {
  ClientConnection *conn = static_cast<ClientConnection *>(param);

  try {
    conn->monitorThreadProcedure();
  } catch (const std::exception &e) {
    std::cerr << "Error in Monitor Thread Procedure: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown error in MonitorThreadProcedure" << std::endl;
  }

  delete conn;
  return nullptr;
}

bool ClientConnection::acceptClient(int listeningSocket) {
  socklen_t clientLength = sizeof(m_clientAddress);
  m_clientSocket = accept(listeningSocket, (struct sockaddr *)&m_clientAddress, &clientLength);

  if (m_clientSocket < 0) {
    std::cerr << "Failed to accept client connection" << std::endl;
    return false;
  }

  try {
    if (m_clientName.empty()) {
      m_clientName = getClientAddress();
    }

    if (pthread_create(&m_monitorThreadId, nullptr, monitorThreadWrapper, this) < 0) {
      throw std::runtime_error("Error creating monitor thread");
    }

    if (pthread_detach(m_monitorThreadId) != 0) {
      throw std::runtime_error("Error detatching monitor-thread");
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    close(m_clientSocket);
    return false;
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

void ClientConnection::setClientPort(int port) {
  m_clientPort = port;
}

bool ClientConnection::isConnected() {
  return m_isConnected;
}
