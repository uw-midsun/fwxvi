/************************************************************************************************
 * @file   client_interface.cc
 *
 * @brief  C++ interface file for C applications
 *
 * @date   2025-03-09
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <cstring>
#include <string>

/* Inter-component Headers */
#include "app_callback.h"
#include "client.h"

/* Intra-component Headers */
#include "client_interface.h"

extern "C" {

void *client_cpp_create(const char *ip_address, int port, void (*msg_callback)(void *, const char *, int), void (*connect_callback)(void *), void *context) {
  Client *client = NULL;
  try {
    std::function<void(Client *, std::string &)> message_callback;
    std::function<void(Client *)> conn_callback;

    if (msg_callback) {
      message_callback = [context, msg_callback](Client *, std::string &message) { msg_callback(context, message.c_str(), static_cast<int>(message.length())); };
    } else {
      message_callback = applicationMessageCallback;
    }

    if (connect_callback) {
      conn_callback = [context, connect_callback](Client *) { connect_callback(context); };
    } else {
      conn_callback = applicationConnectCallback;
    }

    client = new Client(ip_address, port, message_callback, conn_callback);

    return static_cast<void *>(client);
  } catch (const std::exception &e) {
    return NULL;
  }
}

int client_cpp_connect(void *cpp_client) {
  if (cpp_client == NULL) {
    return -1;
  }

  try {
    Client *client = static_cast<Client *>(cpp_client);

    client->connectServer();
    return 0;
  } catch (std::exception &e) {
    return -2;
  }
}

int client_cpp_send_message(void *cpp_client, const char *message, int length) {
  if (cpp_client == NULL || message == NULL) {
    return -1;
  }

  try {
    Client *client = static_cast<Client *>(cpp_client);

    std::string msg(message, length);

    client->sendMessage(msg);
    return 0;
  } catch (std::exception &e) {
    return -2;
  }
}

int client_cpp_disconnect(void *cpp_client) {
  if (cpp_client == NULL) {
    return -1;
  }

  try {
    Client *client = static_cast<Client *>(cpp_client);

    client->disconnectServer();
    return 0;
  } catch (std::exception &e) {
    return -2;
  }
}

void client_cpp_destroy(void *cpp_client) {
  if (cpp_client) {
    Client *client = static_cast<Client *>(cpp_client);
    delete client;
  }
}
}
