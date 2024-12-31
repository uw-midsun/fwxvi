#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <arpa/inet.h>
#include <pthread.h>

#include <atomic>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "client_connection.h"

class TCPServer {
 private:
  using serverCallback = std::function<void(TCPServer *srv, ClientConnection *src, std::string &)>;
  pthread_mutex_t m_mutex;
  serverCallback m_serverCallback;
  std::unordered_map<std::string, ClientConnection *> m_connections;

  std::atomic<bool> m_isListening;
  int m_listenPort;
  int m_listeningSocket;

  pthread_t m_listenThreadId;
  struct sockaddr_in m_serverAddress;

 public:
  TCPServer();
  ~TCPServer();

  void listenThreadProcedure();

  void stop();
  void listenClients(int port, serverCallback callback);
  void messageReceived(ClientConnection *src, std::string &message);

  void sendMessage(ClientConnection *dst, const std::string &message);
  void broadcastMessage(const std::string &message);
  void updateClientName(ClientConnection *conn, std::string newName);
  void removeClient(ClientConnection *conn);
  ClientConnection *getClientByName(std::string &clientName);

  void dumpClientList();
};

#endif
