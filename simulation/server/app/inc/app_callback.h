#ifndef APP_CALLBACK_H
#define APP_CALLBACK_H

#include <string>

#include "client_connection.h"
#include "tcp_server.h"

void applicationCallback(TCPServer *srv, ClientConnection *src, std::string &message);

#endif
