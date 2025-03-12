#pragma once

/************************************************************************************************
 * @file   client_connection.h
 *
 * @brief  Header file defining the ClientConnection class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <atomic>
#include <string>

/* Inter-component Headers */
#include <arpa/inet.h>
#include <pthread.h>

/* Intra-component Headers */

/**
 * @defgroup Server_Utils
 * @brief    Server Utilities and Infrastructure
 * @{
 */

/* Forward declaration */
class Server;

/**
 * @class   ClientConnection
 * @brief   Class that represents a connection between the server and a client
 * @details This class is responsible for managing a single client connection,
 *          monitoring client activity, and sending messages between the server and the client
 */
class ClientConnection {
 private:
  std::atomic<bool> m_isConnected;    /**< Atomic flag indicating whether the client is connected */
  int m_clientPort;                   /**< The clients port which it is connected on */
  int m_clientSocket;                 /**< The clients file descriptor (FD) */
  struct sockaddr_in m_clientAddress; /**< The clients address */
  std::string m_clientName;           /**< The clients name */

  Server *server; /**< Pointer to the server instance */

 public:
  /**
   * @brief   Constructs a ClientConnection object
   * @details Initializes the client connection with the server. The constructor sets up internal
   *          variables and prepares the client to accept and communicate
   * @param   server The server that this client will be connected to
   */
  ClientConnection(Server *server);

  /**
   * @brief   Destructs a ClientConnection object
   * @details If using TCP this closes the existing socket connection
   */
  ~ClientConnection();

  /**
   * @brief   Accepts a client on the listening socket
   * @details This sets the client socket as a non-blocking FD. The server shall add the
   *          client socket to the EPOLL list for listening
   * @param   listeningSocket The listening sockets FD
   */
  bool acceptClient(int listeningSocket);

  /**
   * @brief   Sends a message to the client
   * @param   message String message to be sent
   */
  void sendMessage(const std::string &message);

  /**
   * @brief   Gets the clients name
   * @return  The clients name
   */
  std::string getClientName() const;

  /**
   * @brief   Sets the clients name
   * @param   name The new client name
   */
  void setClientName(const std::string &name);

  /**
   * @brief   Gets the clients port
   * @return  The clients port
   */
  int getClientPort() const;

  /**
   * @brief   Gets the clients socket FD
   * @return  The clients socket FD
   */
  int getSocketFd() const;

  /**
   * @brief   Gets the clients address
   * @return  The clients address
   */
  std::string getClientAddress() const;

  /**
   * @brief   Gets the clients connection status
   * @return  TRUE if the client is connected
   *          FALSE if the client is disconnected
   */
  bool isConnected();
};

/** @} */
