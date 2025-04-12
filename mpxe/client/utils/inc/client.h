#pragma once

/************************************************************************************************
 * @file   client.h
 *
 * @brief  Header file defining the Client class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <atomic>
#include <functional>
#include <queue>
#include <string>

/* Inter-component Headers */
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <unistd.h>

/* Intra-component Headers */

/**
 * @defgroup Client_Utils
 * @brief    Client Utilities and Infrastructure
 * @{
 */

/**
 * @class   Client
 * @brief   Class that represents a client that connects to a main server
 * @details This class is responsible for managing the client connection, binding to the server,
 *          monitoring server activity, and sending messages between the server and the client
 */
class Client {
 private:
  /** @brief  The message callback function definition */
  using messageCallback = std::function<void(Client *client, std::string &)>;
  /** @brief  The connection callback function definition */
  using connectCallback = std::function<void(Client *client)>;

  static constexpr size_t MAX_BUFFER_SIZE = 256; /**< Maximum permitted read size for all clients */

  pthread_t m_receiverThreadId;       /**< Thread Id for reading incoming server data */
  pthread_t m_processMessageThreadId; /**< Thread Id for processing cached server data */
  pthread_mutex_t m_mutex;            /**< Mutex to protect m_messageQueue */
  sem_t m_messageSemaphore;           /**< Semaphore to signal the processMessageThread when new data is received */

  messageCallback m_messageCallback; /**< Function pointer to store the message callback */
  connectCallback m_connectCallback; /**< Function pointer to store the connection callback */

  std::atomic<bool> m_isConnected{ false }; /**< Boolean flag to indicate the servers status */

  std::queue<std::string> m_messageQueue; /**< Queue to cache and input-buffer received server data */

  int m_clientSocket;                 /**< The clients socket FD */
  std::string m_host;                 /**< The servers host address (ie: 127.0.0.1) */
  int m_port;                         /**< The clients port to connect on */
  struct sockaddr_in m_serverAddress; /**< The servers address */

 public:
  /**
   * @brief   Constructs a Client object
   * @details Initializes the client. The constructor sets up internal
   *          variables and prepares the client to accept and communicate
   *          This shall set the host address, listening port and callback functions
   * @param   host Server address to connect to
   * @param   port Port for the client to listen on
   * @param   messageCallback Function pointer to a message callback
   * @param   connectCallback Function pointer to a connection callback
   */
  Client(const std::string &host, int port, messageCallback messageCallback, connectCallback connectCallback);

  /**
   * @brief   Destructs a Client object
   * @details If using TCP this closes the existing socket connection
   *          This shall also delete the mutex, and terminate both running threads
   */
  ~Client();

  /**
   * @brief   Thread procedure for caching/receiving incoming server data
   * @details This thread shall be blocked while no new server data is available
   */
  void receiverProcedure();

  /**
   * @brief   Thread procedure for processing cached/stored server data
   * @details This thread shall be blocked while no new server data is available in storage
   *          Upon receiving a message, the message callback shall be called
   */
  void processMessagesProcedure();

  /**
   * @brief   Connect to the server
   * @details This shall throw an exception if the server does not exist or is not accepting clients
   *          Upon connection, this shall spawn the receiverProcedure and processMessages threads
   */
  void connectServer();

  /**
   * @brief   Disconnect from the server
   * @details If the server is connected, this shall safely terminate both receiverProcedure and processMessages threads
   *          This shall close the existing socket connection
   */
  void disconnectServer();

  /**
   * @brief   Get the connection status of the clienet
   * @return  TRUE if the client is connected
   *          FALSE if the client is disconnected
   */
  bool isConnected() const;

  /**
   * @brief   Function wrapper to transmit a message
   * @param   message String message value to be sent
   */
  void sendMessage(const std::string &message);
};

/** @} */
