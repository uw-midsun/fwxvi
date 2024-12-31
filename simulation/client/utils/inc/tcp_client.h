#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <functional>
#include <iostream>
#include <queue>
#include <string>

class TCPClient {
 private:
  static constexpr size_t MAX_BUFFER_SIZE = 256;
  using messageCallback = std::function<void(TCPClient *client, std::string &)>;
  using connectCallback = std::function<void(TCPClient *client)>;
  int m_clientSocket;
  std::string m_host;
  int m_port;
  struct sockaddr_in m_serverAddress;
  messageCallback m_messageCallback;
  connectCallback m_connectCallback;
  std::queue<std::string> m_messageQueue;

  pthread_t m_receiverThreadId;
  pthread_t m_processMessageThreadId;
  pthread_mutex_t m_mutex;
  sem_t m_messageSemaphore;

  std::atomic<bool> m_isRunning{false};
  std::atomic<bool> m_isConnected{false};

  bool waitForSocket(int timeout_ms, bool read);

 public:
  TCPClient(const std::string &host, int port, messageCallback messageCallback, connectCallback connectCallback);
  ~TCPClient();

  void connectServer();
  void disconnectServer();
  bool isConnected() const;

  void receiverProcedure();
  void processMessages();

  void sendMessage(const std::string &message);
  std::string receiveMessage();
};

#endif
