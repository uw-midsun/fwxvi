#pragma once

/************************************************************************************************
 * @file   server.h
 *
 * @brief  Header file defining the Server class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <atomic>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

/* Inter-component Headers */
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/epoll.h>

/* Intra-component Headers */
#include "client_connection.h"

/**
 * @defgroup Server_Utils
 * @brief    Server Utilities and Infrastructure
 * @{
 */

/**
 * @class   Server
 * @brief   Class that represents the central server that connects to multiple clients
 * @details This class is responsible for managing all clients connections, accepting connections,
 *          monitoring client activity, and sending messages between the server and the client
 */
class Server {
 private:
  /** @brief  The message callback function definition */
  using messageCallback = std::function<void(Server *srv, ClientConnection *src, std::string &)>;
  /** @brief  The connection callback function definition */
  using connectCallback = std::function<void(Server *srv, ClientConnection *src)>;

  static const constexpr unsigned int MAX_SERVER_EPOLL_EVENTS = 64U; /**< Maximum permitted EPOLL events for tracking clients */
  static const constexpr size_t MAX_CLIENT_READ_SIZE = 256U;         /**< Maximum permitted read size for all clients */

  pthread_t m_listenNewClientsId; /**< Thread Id for listening to new clients */
  pthread_t m_epollClientsId;     /**< Thread Id for reading incoming client data */
  pthread_mutex_t m_mutex;        /**< Mutex to protect m_connections map */

  messageCallback m_messageCallback; /**< Function pointer to store the message callback */
  connectCallback m_connectCallback; /**< Function pointer to store the connection callback */

  std::unordered_map<std::string, ClientConnection *> m_connections; /**< Hash-map to store connections based on their string names */

  std::atomic<bool> m_serverListening; /**< Boolean flag to indicate the servers status */

  int m_listenPort;                   /**< The servers port to listen on */
  int m_listeningSocket;              /**< The servers listening socket FD */
  struct sockaddr_in m_serverAddress; /**< The servers address */

  int m_epollFd;                                             /**< The servers EPOLL FD to manage reading all clients */
  struct epoll_event m_epollEvents[MAX_SERVER_EPOLL_EVENTS]; /**< An array to store all EPOLL events as bitmasks */

 public:
  /**
   * @brief   Constructs a Server object
   * @details Initializes the server. The constructor sets up internal
   *          variables and prepares the server to accept and communicate
   */
  Server();

  /**
   * @brief   Destructs a Server object
   * @details If using TCP this closes all existing socket connections
   *          This shall also delete the mutex, and terminate both running threads
   */
  ~Server();

  /**
   * @brief   Thread procedure for listening for new client connections
   * @details This thread shall be blocked while no new clients are available
   *          Upon connection, the client shall be declared as non-blocking, and will
   *          be added to the EPOLL list as an input FD. Further, the connection callback
   *          shall be called
   */
  void listenNewClientsProcedure();

  /**
   * @brief   Thread procedure for reading incoming client data
   * @details This thread shall be blocked while no new client data is available
   *          Upon receiving a message, the message callback shall be called
   */
  void epollClientsProcedure();

  /**
   * @brief   Stops the server
   * @details This shall terminate all connections safely, and terminate running threads
   */
  void stop();

  /**
   * @brief   Initiate the server to listen to clients on a provided port
   * @details This shall start the listenNewClientsProcedure and epollClientsProcedure
   * @param   port Port for the server to listen on
   * @param   messageCallback Function pointer to a message callback
   * @param   connectCallback Function pointer to a connection callback
   */
  void listenClients(int port, messageCallback messageCallback, connectCallback connectCallback);

  /**
   * @brief   Function wrapper around the message callback
   * @param   client Pointer to the client which has received a message
   * @param   message String message value that has been received
   */
  void messageReceived(ClientConnection *client, std::string &message);

  /**
   * @brief   Function wrapper to transmit a message
   * @param   client Pointer to the client which shall be written to
   * @param   message String message value to be sent
   */
  void sendMessage(ClientConnection *client, const std::string &message);

  /**
   * @brief   Function wrapper to broadcast a message to all clients
   * @param   message String message value to be sent
   */
  void broadcastMessage(const std::string &message);

  /**
   * @brief   Update a clients name
   * @param   client Pointer to the client which shall be updated
   * @param   newName String value of the clients new name
   */
  void updateClientName(ClientConnection *client, std::string newName);

  /**
   * @brief   Remove a client
   * @param   client Pointer to the client which shall be removed
   */
  void removeClient(ClientConnection *client);

  /**
   * @brief   Get a client by its name
   * @param   clientName String value of the clients name
   * @return  Pointer to the respective client connection object
   */
  ClientConnection *getClientByName(std::string &clientName);

  /**
   * @brief   Print a list of all connected clients
   */
  void dumpClientList();
};

/** @} */
