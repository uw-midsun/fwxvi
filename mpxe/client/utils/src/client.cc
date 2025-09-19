<<<<<<< HEAD
/************************************************************************************************
 * @file   client.cc
 *
 * @brief  Source file defining the Client class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <cstring>
#include <iostream>

/* Inter-component Headers */

/* Intra-component Headers */
#include "client.h"

void Client::processMessagesProcedure() {
  while (m_isConnected) {
    /* Wait for new data */
    sem_wait(&m_messageSemaphore);

    pthread_mutex_lock(&m_mutex);

    if (!m_messageQueue.empty()) {
      auto message = m_messageQueue.front();
      m_messageQueue.pop();
      if (m_messageCallback) {
        m_messageCallback(this, message);
      }
    }

    pthread_mutex_unlock(&m_mutex);
  }
}

void Client::receiverProcedure() {
  while (m_isConnected) {
    std::string message(MAX_BUFFER_SIZE, '\0');

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(m_clientSocket, &readSet);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    int selectResult = select(m_clientSocket + 1, &readSet, NULL, NULL, &timeout);

    if (selectResult == -1) {
      if (errno == EINTR) {
        continue;
      }

      disconnectServer();
      throw std::runtime_error("Select error: " + std::string(strerror(errno)));
    } else if (selectResult > 0 && FD_ISSET(m_clientSocket, &readSet)) {
      size_t bytesRead = read(m_clientSocket, &message[0], message.length());

      if (bytesRead <= 0) {
        disconnectServer();
        throw std::runtime_error("Connection lost");
      }

      message[bytesRead] = '\0';

      pthread_mutex_lock(&m_mutex);
      m_messageQueue.push(message);
      pthread_mutex_unlock(&m_mutex);

      /* Signal that there is new data */
      sem_post(&m_messageSemaphore);
    }
  }
}

void *processMessagesProcedureWrapper(void *param) {
  Client *client = static_cast<Client *>(param);

  try {
    client->processMessagesProcedure();
  } catch (std::exception &e) {
    std::cerr << "Process Messages Thread Error " << e.what() << std::endl;
  }

  return nullptr;
}

void *receiverProcedureWrapper(void *param) {
  Client *client = static_cast<Client *>(param);

  try {
    client->receiverProcedure();
  } catch (std::exception &e) {
    std::cerr << "Receiver Thread Error " << e.what() << std::endl;
  }

  return nullptr;
}

void Client::connectServer() {
  try {
    m_clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (m_clientSocket < 0) throw std::runtime_error("Error created socket");

    m_serverAddress.sin_family = AF_INET;
    m_serverAddress.sin_port = htons(m_port);

    if (inet_pton(AF_INET, m_host.c_str(), &m_serverAddress.sin_addr) <= 0) {
      close(m_clientSocket);
      throw std::runtime_error("Error converting IPv4 host address to binary form");
    }

    if (connect(m_clientSocket, (struct sockaddr *)&m_serverAddress, sizeof(m_serverAddress)) < 0) {
      close(m_clientSocket);
      throw std::runtime_error("Error connecting socket");
    }

    m_isConnected = true;

    if (m_connectCallback) {
      m_connectCallback(this);
    }

    if (pthread_create(&m_receiverThreadId, NULL, receiverProcedureWrapper, this)) {
      close(m_clientSocket);
      throw std::runtime_error("Failed to create receiver thread");
    }

    if (pthread_create(&m_processMessageThreadId, NULL, processMessagesProcedureWrapper, this)) {
      close(m_clientSocket);
      throw std::runtime_error("Failed to create process messages thread");
    }
  } catch (std::exception &e) {
    std::cerr << "Error connecting to the server: " << e.what() << std::endl;
  }
}

void Client::disconnectServer() {
  m_isConnected = false;
  close(m_clientSocket);
  m_clientSocket = 0;
}

void Client::sendMessage(const std::string &message) {
  int n = send(m_clientSocket, message.c_str(), message.size(), 0);
  if (n < 0) throw std::runtime_error("Error sending message");
}

bool Client::isConnected() const {
  return m_isConnected;
}

void Client::setClientName(const std::string &name) {
  this->m_clientName = name;
}

std::string Client::getClientName() const {
  return this->m_clientName;
}

Client::Client(const std::string &host, int port, messageCallback messageCallback, connectCallback connectCallback) {
  this->m_host = host;
  this->m_port = port;
  this->m_clientSocket = -1;
  this->m_messageCallback = messageCallback;
  this->m_connectCallback = connectCallback;

  if (pthread_mutex_init(&m_mutex, NULL) != 0) {
    throw std::runtime_error("Error initializing mutex");
  }

  if (sem_init(&m_messageSemaphore, 0, 0) != 0) {
    throw std::runtime_error("Error initializing semaphore");
  }
}

Client::~Client() {
  disconnectServer();
  pthread_mutex_destroy(&m_mutex);
  sem_destroy(&m_messageSemaphore);
}
>>>>>>> origin/main
