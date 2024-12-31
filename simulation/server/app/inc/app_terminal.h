#ifndef APP_TERMINAL_H
#define APP_TERMINAL_H

#include "client_connection.h"
#include "tcp_server.h"
class Terminal {
 private:
  TCPServer *m_Server;
  ClientConnection *m_targetClient;

  std::string toLower(const std::string &input);

  void handleGpioCommands(const std::string &action, std::vector<std::string> &tokens);
  void parseCommand(std::vector<std::string> &tokens);

 public:
  Terminal(TCPServer *server);

  void run();
};

#endif
