#include "tcp_client.h"

#include <cstring>
#include <iostream>

#define MAX_PACKET_SIZE 256U

bool TCPClient::waitForSocket(int timeout_ms, bool read) {
  struct pollfd fds[1];
  fds[0].fd = m_clientSocket;
  fds[0].events = read ? POLLIN : POLLOUT;

  int result = poll(fds, 1, timeout_ms);

  if (result < 0) {
    return false;
  }

  return result > 0;
}

void TCPClient::processMessages() {
  while (m_isRunning) {
    /* Wait for new data */
    sem_wait(&m_messageSemaphore);

    pthread_mutex_lock(&m_mutex);

    if (!m_messageQueue.empty()) {
      auto message = m_messageQueue.front();
      m_messageQueue.pop();

      if (m_messageCallback) {
        m_messageCallback(this, message);
      }
      pthread_mutex_unlock(&m_mutex);
    }
  }
}

void TCPClient::receiverProcedure() {
  while (m_isRunning) {
    /* wait for socket to be notified */
    if (!waitForSocket(1000, true)) {
      continue;
    }

    std::string message(MAX_PACKET_SIZE, '\0');

    size_t bytesRead = read(m_clientSocket, &message[0], sizeof(message));

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

void *processMessagesThread(void *param) {
  TCPClient *client = static_cast<TCPClient *>(param);

  try {
    client->processMessages();
  } catch (...) {
    std::cerr << "Process Messages Thread Error" << std::endl;
  }

  return nullptr;
}

void *receiverThreadProcedure(void *param) {
  TCPClient *client = static_cast<TCPClient *>(param);

  try {
    client->receiverProcedure();
  } catch (...) {
    std::cerr << "Receiver Thread Error" << std::endl;
  }

  return nullptr;
}

void TCPClient::connectServer() {
  m_clientSocket = socket(AF_INET, SOCK_STREAM, 0);

  if (m_clientSocket < 0)
    throw std::runtime_error("Error created socket");

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

  if (!waitForSocket(5000, false)) {
    close(m_clientSocket);
    throw std::runtime_error("Connection timeout");
  }

  m_isRunning = true;
  m_isConnected = true;

  if (m_connectCallback) {
    m_connectCallback(this);
  }

  if (pthread_create(&m_receiverThreadId, NULL, receiverThreadProcedure, this)) {
    close(m_clientSocket);
    throw std::runtime_error("Failed to create receiver thread");
  }

  if (pthread_create(&m_processMessageThreadId, NULL, processMessagesThread, this)) {
    close(m_clientSocket);
    throw std::runtime_error("Failed to create process messages thread");
  }

  std::cout << "Connected :-)" << std::endl;
}

void TCPClient::disconnectServer() {
  close(m_clientSocket);
  m_clientSocket = 0;
}

void TCPClient::sendMessage(const std::string &message) {
  int n = send(m_clientSocket, message.c_str(), message.size(), 0);
  if (n < 0)
    throw std::runtime_error("Error sending message");
}

std::string TCPClient::receiveMessage() {
  std::string message(MAX_PACKET_SIZE, '\0');

  int n = read(m_clientSocket, &message[0], MAX_PACKET_SIZE);
  message[n] = '\0';

  return message;
}

bool TCPClient::isConnected() const {
  return m_isConnected;
}

TCPClient::TCPClient(const std::string &host, int port, messageCallback messageCallback, connectCallback connectCallback) {
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

TCPClient::~TCPClient() {
  disconnectServer();
  pthread_mutex_destroy(&m_mutex);
  sem_destroy(&m_messageSemaphore);
}
