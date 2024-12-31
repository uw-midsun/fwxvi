#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include <arpa/inet.h>
#include <pthread.h>

#include <atomic>
#include <string>

/* Forward declaration */
class TCPServer;

class ClientConnection {
 private:
  static const constexpr size_t MAX_CLIENT_BUFFER_SIZE = 512U;

  std::atomic<bool> m_isConnected;
  int m_clientPort;
  int m_clientSocket;
  struct sockaddr_in m_clientAddress;
  pthread_t m_monitorThreadId;
  std::string m_clientName;

  TCPServer *server;

  std::string getClientAddress() const;

 public:
  ClientConnection(TCPServer *server);
  ~ClientConnection();

  bool acceptClient(int listeningSocket);

  void monitorThreadProcedure();
  void sendMessage(const std::string &message);

  std::string getClientName() const;
  void setClientName(const std::string &name);

  int getClientPort() const;
  void setClientPort(int port);

  bool isConnected();
};

#endif
