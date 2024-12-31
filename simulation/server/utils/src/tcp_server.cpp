#include "tcp_server.h"

#include <string.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

TCPServer::TCPServer() {
  m_isListening = false;
  m_listenPort = -1;
  pthread_mutex_init(&m_mutex, nullptr);
}

TCPServer::~TCPServer() {
  pthread_mutex_destroy(&m_mutex);
}

void TCPServer::listenThreadProcedure() {
  m_listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (m_listeningSocket < 0) {
    throw std::runtime_error("Error creating socket for port " + std::to_string(m_listenPort));
  }

  int enable = 1;
  if (setsockopt(m_listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    throw std::runtime_error("Error setting socket option SO_REUSEADDR");
  }
  memset((char *)&m_serverAddress, 0U, sizeof(m_serverAddress));
  m_serverAddress.sin_family = AF_INET;
  m_serverAddress.sin_port = htons(m_listenPort);
  m_serverAddress.sin_addr.s_addr = INADDR_ANY; /* Listen to all addresses */

  if (bind(m_listeningSocket, (struct sockaddr *)&m_serverAddress, sizeof(m_serverAddress)) < 0) {
    throw std::runtime_error("Error binding socket");
  }

  listen(m_listeningSocket, 5);
  m_isListening = true;

  while (m_isListening) {
    ClientConnection *conn = new ClientConnection(this);

    if (!conn->acceptClient(m_listeningSocket)) {
      delete conn;
      break;
    }

    updateClientName(conn, conn->getClientName());
  }

  m_isListening = false;
}

void *listenThreadWrapper(void *param) {
  TCPServer *server = static_cast<TCPServer *>(param);

  try {
    server->listenThreadProcedure();
  } catch (...) {
    std::cerr << "Listen Thread Error" << std::endl;
  }

  return nullptr;
}

void TCPServer::listenClients(int port, serverCallback callback) {
  if (m_isListening)
    return;

  m_listenPort = port;
  m_serverCallback = callback;

  if (pthread_create(&m_listenThreadId, nullptr, listenThreadWrapper, this)) {
    throw std::runtime_error("Listen Error");
  }
}

void TCPServer::removeClient(ClientConnection *conn) {
  if (conn) {
    std::cerr << "Removed client: " << conn->getClientName() << std::endl;
    m_connections.erase(conn->getClientName());
  }
}

void TCPServer::updateClientName(ClientConnection *conn, std::string newName) {
  pthread_mutex_lock(&m_mutex);
  std::string oldClientName = conn->getClientName();

  if (!oldClientName.empty()) {
    m_connections.erase(oldClientName);
  }

  conn->setClientName(newName);

  /* Handle potential client duplicates */
  std::string clientName = newName;
  unsigned int clientDuplicateCount = 1U;
  while (m_connections.count(clientName)) {
    clientName = newName + " (" + std::to_string(clientDuplicateCount) + ")";
    clientDuplicateCount++;
  }

  m_connections[clientName] = conn;
  conn->setClientName(clientName);
  pthread_mutex_unlock(&m_mutex);
}

void TCPServer::messageReceived(ClientConnection *src, std::string &message) {
  m_serverCallback(this, src, message);
}

void TCPServer::sendMessage(ClientConnection *dst, const std::string &message) {
  dst->sendMessage(message);
}

void TCPServer::broadcastMessage(const std::string &message) {
  for (auto &pair : m_connections) {
    if (pair.second->isConnected()) {
      pair.second->sendMessage(message);
    }
  }
}

ClientConnection *TCPServer::getClientByName(std::string &clientName) {
  if (m_connections.count(clientName)) {
    return m_connections[clientName];
  }
  return nullptr;
}

void TCPServer::dumpClientList() {
  for (auto &pair : m_connections) {
    if (pair.second->isConnected()) {
      std::cout << pair.first << std::endl;
    }
  }
}

void TCPServer::stop() {}
